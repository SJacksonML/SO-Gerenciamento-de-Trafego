#define MAX_VEHICLE 30

int VehiclesCurrently = 0;

typedef enum{
    TYPE_AMBULANCE,
    TYPE_CAR,
}VehicleType;

typedef enum{
    SPEED_SLOW,
    SPEED_MEDIUM,
    SPEED_FAST,
}Speed;

typedef struct Pos Pos;

typedef struct Vehicle Vehicle;
