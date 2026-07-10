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

    /* Estado do gerador de números aleatórios PRÓPRIO deste veículo.
     * Necessário porque rand()/srand() usam estado global e não são
     * thread-safe: com várias threads de veículo chamando rand() ao
     * mesmo tempo (ex.: para decidir a direção num cruzamento) ocorre
     * condição de corrida. Cada veículo usa rand_r(&rng_state), que é
     * thread-safe pois opera sobre memória local à própria struct. */
    unsigned int rng_state;
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