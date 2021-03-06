#include "PseuWoW.h"
#include "WorldSession.h"
#include "World.h"
#include "MovementMgr.h"
#include "Player.h"
#include "MovementInfo.h"
#include <math.h>

MovementMgr::MovementMgr()
{
    _moveFlags = 0;
    _instance = NULL;
    _optime = 0;
    lastUpdateTime = 0;
    heartbeatEnabled = true;
    _moved = false;
    walkingToTarget = false;
}

MovementMgr::~MovementMgr()
{
}


void MovementMgr::SetInstance(PseuInstance *inst)
{
    //_movemode = MOVEMODE_MANUAL;
    _movemode = MOVEMODE_AUTO;
    _instance = inst;
    _mychar = inst->GetWSession()->GetMyChar();
    if(!_mychar)
    {
        logerror("MovementMgr: MyCharacter doesn't exist!");
        // it will likely crash somewhere after outputting this message, but in case it really appears,
        // we can be sure there is something absolutely not ok...
    }
}

void MovementMgr::_BuildPacket(uint16 opcode)
{
    WorldPacket *wp = new WorldPacket(opcode,4+2+4+16); // it can be larger, if we are jumping, on transport or swimming
    wp->appendPackGUID(_mychar->GetGUID());
    MovementInfo mi;
    mi.SetMovementFlags(_moveFlags);
    mi.time = getMSTime();
    mi.pos = _mychar->GetPosition();
    mi.fallTime = _falltime;
    *wp << mi;

    // TODO: transport not yet handled/done
    // TODO: swimming not yet done
    // TODO: jumping not done yet
    // TODO: spline not yet done

    DEBUG(logdebug("Move flags: 0x%X (packet: %u bytes)",_moveFlags,wp->size()));
    // send the packet, threadsafe
    _instance->GetWSession()->AddSendWorldPacket(wp);
    _moved = true;
    _optime = getMSTime();
    _falltime = 0; //HACK!!!!


}

void MovementMgr::Update(bool sendDirect)
{
    uint32 curtime = getMSTime();
    uint32 timediff = curtime - lastUpdateTime;
    lastUpdateTime = curtime;

    WorldPosition pos = _mychar->GetPosition();
//    float turnspeed = _mychar->GetSpeed(MOVE_TURN) / 1000.0f * timediff;
    float totalDistance = _mychar->GetSpeed(MOVE_RUN) * (timediff / 1000.0f);
    // or use walkspeed, depending on setting. for now use only runspeed
    // TODO: calc other speeds as soon as implemented
/*
    if(_movemode == MOVEMODE_MANUAL)
    {
        if(_moveFlags & MOVEMENTFLAG_JUMPING)
        {
            // approx. jumping formula
            _jumptime += timediff / 1000.0f;
            if (_jumptime < 0.4f)
                pos.z += 2.0f / 1000.0f * timediff;
            if (_jumptime >= 0.4f && _jumptime < 0.8f)
            {
                pos.z -= 2.0f / 1000.0f * timediff;
            }
            if (_jumptime >= 0.8f)
            {
                _jumptime = 0.0f;
                _moveFlags &= ~MOVEMENTFLAG_JUMPING;
            }
        } */

        if(_movemode == MOVEMODE_AUTO)
        {
            if(_moveFlags & MOVEMENTFLAG_FORWARD)
            {

                //WorldPosition oldpos = pos;
                pos.x += totalDistance * sin(pos.o + (M_PI/2));
                pos.y -= totalDistance * cos(pos.o + (M_PI/2));
//                if (_instance->GetWSession()->GetWorld()->GetPosZ(pos.x,pos.y) > 5.0f + pos.z)
//                {
//                    pos = oldpos;
//                }
            }
            if(_moveFlags & MOVEMENTFLAG_BACKWARD)
            {

                WorldPosition oldpos = pos;
                pos.x -= totalDistance * sin(pos.o + (M_PI/2));
                pos.y += totalDistance * cos(pos.o + (M_PI/2));
//                if (_instance->GetWSession()->GetWorld()->GetPosZ(pos.x,pos.y) > 5.0f + pos.z)
//                {
//                    pos = oldpos;
//                }
            }
            _mychar->SetPosition(pos);
        }

    // if we are moving, and 500ms have passed, send an heartbeat packet.
        //just in case 500ms have passed but the packet is sent by another function, do not send here
    //if( !sendDirect && (_moveFlags & MOVEMENTFLAG_ANY_MOVE_NOT_TURNING) && _optime + MOVE_HEARTBEAT_DELAY < getMSTime())
    if(heartbeatEnabled && !sendDirect && (_moveFlags > 0) && _optime + MOVE_HEARTBEAT_DELAY < getMSTime()) //HEARTBEAT
    {
        _BuildPacket(MSG_MOVE_HEARTBEAT);

        // also need to tell the world map mgr that we moved; maybe maps need to be loaded
        // the main thread will take care of really loading the maps; here we just tell our updated position
        if(World *world = _instance->GetWSession()->GetWorld())
        {
            world->UpdatePos(pos.x, pos.y, world->GetMapId());
        }
    }
    // TODO: apply gravity, handle falling, swimming, etc.
    static bool firstTarget = true;
    static WorldPosition first(-8358.f, 723.f, 59.45);
    static WorldPosition nfirst(-8348.f, 723.f, 59.45);
    if (!sendDirect) { //ignore update calls that came not from the world update cycle
        if (firstTarget == true) {
            walkStraightToTarget(first);
            if (!walkingToTarget) {
                firstTarget = false;
            }
        } else {
            walkStraightToTarget(nfirst);
            if (!walkingToTarget) {
                firstTarget = true;
            }
        }
    }
}

// stops
void MovementMgr::MoveStop(void)
{
    if(!(_moveFlags & (MOVEMENTFLAG_FORWARD | MOVEMENTFLAG_BACKWARD)))
        return;
    _moveFlags &= ~(MOVEMENTFLAG_FORWARD | MOVEMENTFLAG_BACKWARD | MOVEMENTFLAG_WALK_MODE);
    Update(true);
    _BuildPacket(MSG_MOVE_STOP);
}

void MovementMgr::MoveFallLand(void)
{
    Update(true);
    _BuildPacket(MSG_MOVE_FALL_LAND);
}

void MovementMgr::MoveStartForward(void)
{
    if(_moveFlags & MOVEMENTFLAG_FORWARD)
        return;
    _moveFlags |= MOVEMENTFLAG_FORWARD;
    _moveFlags &= ~MOVEMENTFLAG_BACKWARD;
    Update(true);
    _BuildPacket(MSG_MOVE_START_FORWARD);
}

void MovementMgr::MoveStartBackward(void)
{
    if(_moveFlags & MOVEMENTFLAG_BACKWARD)
        return;
    _moveFlags |= (MOVEMENTFLAG_BACKWARD | MOVEMENTFLAG_WALK_MODE);
    // backward walk is always slow; flag must be set, otherwise causing weird movement in other client
    _moveFlags &= ~MOVEMENTFLAG_FORWARD;
    Update(true);
    _BuildPacket(MSG_MOVE_START_BACKWARD);
}

void MovementMgr::MoveStartStrafeLeft(void)
{
    if(_moveFlags & MOVEMENTFLAG_STRAFE_LEFT)
        return;
    _moveFlags |= MOVEMENTFLAG_STRAFE_LEFT;
    _moveFlags &= ~MOVEMENTFLAG_STRAFE_RIGHT;
    Update(true);
    _BuildPacket(MSG_MOVE_START_STRAFE_LEFT);
}

void MovementMgr::MoveStartStrafeRight(void)
{
    if(_moveFlags & MOVEMENTFLAG_STRAFE_RIGHT)
        return;
    _moveFlags |= MOVEMENTFLAG_STRAFE_RIGHT;
    _moveFlags &= ~MOVEMENTFLAG_STRAFE_LEFT;
    Update(true);
    _BuildPacket(MSG_MOVE_START_STRAFE_RIGHT);
}

void MovementMgr::MoveStopStrafe(void)
{
    if(!(_moveFlags & (MOVEMENTFLAG_STRAFE_RIGHT | MOVEMENTFLAG_STRAFE_LEFT)))
        return;
    _moveFlags &= ~(MOVEMENTFLAG_STRAFE_RIGHT | MOVEMENTFLAG_STRAFE_LEFT);
    Update(true);
    _BuildPacket(MSG_MOVE_START_STRAFE_RIGHT);
}


void MovementMgr::MoveStartTurnLeft(void)
{
    if(_moveFlags & MOVEMENTFLAG_TURN_LEFT)
        return;
    _moveFlags |= MOVEMENTFLAG_TURN_LEFT;
    _moveFlags &= ~MOVEMENTFLAG_TURN_RIGHT;
    Update(true);
    _BuildPacket(MSG_MOVE_START_TURN_LEFT);
}

void MovementMgr::MoveStartTurnRight(void)
{
    if(_moveFlags & MOVEMENTFLAG_TURN_RIGHT)
        return;
    _moveFlags |= MOVEMENTFLAG_TURN_RIGHT;
    _moveFlags &= ~MOVEMENTFLAG_TURN_LEFT;
    Update(true);
    _BuildPacket(MSG_MOVE_START_TURN_RIGHT);
}

void MovementMgr::MoveStopTurn(void)
{
    if(!(_moveFlags & (MOVEMENTFLAG_TURN_LEFT | MOVEMENTFLAG_TURN_RIGHT)))
        return;
    _moveFlags &= ~(MOVEMENTFLAG_TURN_LEFT | MOVEMENTFLAG_TURN_RIGHT);
    Update(true);
    _BuildPacket(MSG_MOVE_STOP_TURN);
}

void MovementMgr::MoveSetFacing(void)
{
    Update(true);
    _BuildPacket(MSG_MOVE_SET_FACING);
}

void MovementMgr::MoveJump(void)
{
    if(!(_moveFlags & (MOVEMENTFLAG_FALLING | MOVEMENTFLAG_PENDINGSTOP)))
        return;
    _moveFlags |= MOVEMENTFLAG_FALLING;
    Update(true);
    _BuildPacket(MSG_MOVE_JUMP);
}

bool MovementMgr::IsMoving(void)
{
    return _moveFlags & MOVEMENTFLAG_ANY_MOVE;
}

bool MovementMgr::IsTurning(void)
{
    return _moveFlags & (MOVEMENTFLAG_TURN_LEFT | MOVEMENTFLAG_TURN_RIGHT);
}

bool MovementMgr::IsWalking(void)
{
    return _moveFlags & (MOVEMENTFLAG_FORWARD | MOVEMENTFLAG_BACKWARD);
}

bool MovementMgr::IsStrafing(void)
{
    return _moveFlags & (MOVEMENTFLAG_STRAFE_LEFT | MOVEMENTFLAG_STRAFE_RIGHT);
}

void MovementMgr::walkStraightToTarget(WorldPosition target)
{    
    targetPosition = target;
    if (distToTarget() < 0.1) {
        MoveStop();
        walkingToTarget = false;
        return;
    }
    walkingToTarget = true;
    facePoint(target);
    if (!(_moveFlags & MOVEMENTFLAG_FORWARD)) {
        MoveStartForward();
    }
}

void MovementMgr::facePoint(WorldPosition point)
{
    WorldPosition currentPos = _mychar->GetPosition();
    const float diffx = point.x - currentPos.x;
    const float diffy = point.y - currentPos.y;    
    float theta = atan2(diffy, diffx);    
    theta = theta < 0 ? theta + 2*M_PI : theta;    
    if (_mychar->GetPosition().o == theta) {
        return; //already facing that direction
    }
    currentPos.o = theta;
    _mychar->SetPosition(currentPos);    
    MoveSetFacing();    
}

float MovementMgr::distToTarget()
{
    const WorldPosition &cp = _mychar->GetPosition();
    return sqrt(pow(cp.x - targetPosition.x, 2) + \
                pow(cp.y - targetPosition.y, 2) + \
                pow(cp.z - targetPosition.z, 2));
}

