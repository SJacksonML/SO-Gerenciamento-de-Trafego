#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>

#include "../include/vehicle.h"
#include "../include/map.h"
#include "../include/clock.h"
#include "../include/sync.h"

struct Pos{
    int row;
    int col;
};

struct Vehicle{
    int id;
    VehicleType type;
    Direction direction;
    Speed speed;
    Pos pos;
    Map *map;
};

static bool should_move(Vehicle *vehicle){
    if (!vehicle) return false;

    int tick = clock_get_tick(); //conversa necessária, sem acesso ao clock
    switch (vehicle->speed){
        case SPEED_FAST: return true;
        case SPEED_MEDIUM: return (tick % 2 == 0);
        case SPEED_SLOW: return (tick % 4 == 0);
        default: false;
    }
}

static Direction dir_from_tile(Vehicle *vehicle){
    int x = vehicle->pos.col;
    int y = vehicle->pos.row;
    CellType tile_type = vehicle->map->grid[y][x].type;
    switch (tile_type) {
        case CELL_ONE_WAY_N: return DIR_NORTH;
        case CELL_ONE_WAY_S: return DIR_SOUTH;
        case CELL_ONE_WAY_E: return DIR_EAST;
        case CELL_ONE_WAY_W: return DIR_WEST;
        case CELL_INTERSECTION: return DIR_COUNT;
        case CELL_ROAD: return DIR_COUNT;
        default: return DIR_INVALID;
    }
}

static Pos car_next_pos(Vehicle *vehicle){
    Pos next = vehicle->pos;
    Direction dir = vehicle->direction;

    switch (dir){
        DIR_NORTH: next.row--; break;
        DIR_SOUTH: next.row++; break;
        DIR_EAST: next.col--; break;
        DIR_WEST: next.col++; break;
        default: break;
    }
    return next;
}

static bool car_advance(Vehicle *vehicle, Pos next){
    if (!should_move(vehicle)) return false;
    Direction check = map_is_valid_move(vehicle->map, next.row, next.col, vehicle->direction);

    switch (check){
        case DIR_INVALID: break; // * invalid movement

        case DIR_COUNT: break; // *signal/intersection treatment
        /*
        * waiting updates on sync for correct implementation
        *TrafficSignal *TL = sync_get_signal(next.row, next.col); 
        *if (vehicle->type == TYPE_AMBULANCE) traffic_request_priority(next.row, next.col, vehicle->direction);
        *traffic_wait_green(next.row, next.col, vehicle->direction);
        *if (cell_try_occupy(vehicle->map, next.row, next.col, vehicle->id)){
        *    vehicle->pos = next;
        *    cell_release(vehicle->map, next.row, next.col);
        *    return true;
        */
        
        default: // * handling direction-swap tiles
        if (cell_try_occupy(vehicle->map, next.row, next.col, vehicle->id)){
            vehicle->pos = next;
            cell_release(vehicle->map, next.row, next.col);
            vehicle->direction = check;
            return true;
        }
    }
    return false;
}

static void car_wait_green(Vehicle *vehicle, Pos signal){
}

Vehicle *vehicle_init(Vehicle *vehicle, int id, VehicleType t, Direction d, Pos start, Speed s, Map *m){
    vehicle->id = id;
    vehicle->type = t;
    vehicle->direction = d;
    vehicle->pos = start;
    vehicle->speed = s;
    vehicle->map = m;
    return vehicle;
}

void vehicle_run(Vehicle *vehicle){
    int current_tick;
    Map *map = vehicle->map;
    while (1/*change to ticks*/){
        current_tick = clock_get_tick; //conversa necessária, não há acesso ao clock
        Pos pos = vehicle->pos;
        Direction tile_dir = dir_from_tile(vehicle);

        switch (tile_dir){
            case DIR_INVALID: break; // *possible error treatment

            case DIR_COUNT: break; // *waiting for sync updates

            default: Pos next = car_next_pos(vehicle); // *handling the position advancement in direction-swap tiles
            car_advance(vehicle, next);
            }
        clock_wait_tick(); // *waiting for clock to broadcast
        }
}