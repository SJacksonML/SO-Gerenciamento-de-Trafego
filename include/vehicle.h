#include "map.h"

/* número máximo de veículos, deve ser respeitado na implementação para evitar engarrafamentos numerosos */
#define MAX_VEHICLE 20

/* enum dos tipos de veículo */
typedef enum{
    TYPE_AMBULANCE,
    TYPE_CAR,
}VehicleType;

/* enum das possíveis velocidades de um veículo */
typedef enum{
    SPEED_SLOW,
    SPEED_MEDIUM,
    SPEED_FAST,
} Speed;

/* struct de posição atual de um veículo */
typedef struct Pos{
    int row;
    int col;
} Pos;

/* struct de um veículo */
typedef struct Vehicle{
    int id;
    VehicleType type;
    Direction direction;
    Speed speed;
    Pos pos;
    Map *map;
} Vehicle;

/*
*função para alocar dados de um veículo
*args: Vehicle *vehicle, int id, VehicleType t, Direction d, Pos start, Speed s, Map *m
*retorna: vehicle (ponteiro entregue à função que pertence ao veículo)
*/
Vehicle *vehicle_init(Vehicle *vehicle, int id, VehicleType t, Direction d, Pos start, Speed s, Map *m);

/*
*função principal de vida de um veículo, usada para iniciar as threads.
*args: Vehicle *vehicle
*/
void vehicle_run(Vehicle *vehicle);