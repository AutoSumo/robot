#include <stdint.h>

#pragma pack(1)

enum PacketType : uint8_t
{
    MOVE_MOTORS = 1,
    IR_DATA,
    MOVE_SERVO
};

struct MoveMotorsPacket
{
    uint8_t leftDirection;
    uint16_t leftPower;
    uint8_t rightDirection;
    uint16_t rightPower;
};

struct MoveServoPacket
{
    uint8_t angle;
};

struct IRDataPacket
{
    PacketType type;
    uint8_t left;
    uint8_t right;
};