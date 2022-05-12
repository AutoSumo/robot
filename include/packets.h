#include <stdint.h>

#pragma pack(1)

enum PacketType : uint8_t
{
    MOVE_MOTORS = 1
};

struct MoveMotorsPacket
{
    uint8_t leftDirection;
    uint16_t leftPower;
    uint8_t rightDirection;
    uint16_t rightPower;
};