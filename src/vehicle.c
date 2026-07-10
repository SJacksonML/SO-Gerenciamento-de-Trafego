#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

#include "../include/vehicle.h"
#include "../include/map.h"
#include "../include/clock.h"
#include "../include/sync.h"


static bool should_move(Vehicle *vehicle, int tick){
    if (!vehicle) return false;

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
        default: return DIR_INVALID;
    }
}

static Direction switch_dir(Vehicle *vehicle, int l_or_r){
    Direction dir = vehicle->direction;
    if (l_or_r == 1){
        switch (dir){
            case DIR_NORTH: return DIR_EAST;
            case DIR_SOUTH: return DIR_WEST;
            case DIR_WEST: return DIR_NORTH;
            case DIR_EAST: return DIR_SOUTH;
        }
    }
    else{
        switch (dir){
            case DIR_NORTH: return DIR_WEST;
            case DIR_SOUTH: return DIR_EAST;
            case DIR_WEST: return DIR_SOUTH;
            case DIR_EAST: return DIR_NORTH;
        }
    }
}

static Pos car_next_pos(Vehicle *vehicle){
    Pos next = vehicle->pos;
    Direction dir = vehicle->direction;

    switch (dir){
        case DIR_NORTH: next.row--; break;
        case DIR_SOUTH: next.row++; break;
        case DIR_EAST: next.col--; break;
        case DIR_WEST: next.col++; break;
        default: break;
    }
    return next;
}

static bool car_advance(Vehicle *vehicle, Pos next, int tick){
    if (!should_move(vehicle, tick)) return false;
    Direction check = map_is_valid_move(vehicle->map, next.row, next.col, vehicle->direction);
    Pos old_pos = vehicle->pos;

    switch (check){
        case DIR_INVALID: vehicle->direction = switch_dir(vehicle, 2); // *switch directions on wall
        return true;

        case DIR_COUNT: // *signal/intersection treatment
        TrafficSignal *TLThisCell = sync_get_signal(vehicle->pos.row, vehicle->pos.col);

        if (TLThisCell == NULL){
            if (vehicle->type == TYPE_AMBULANCE) traffic_request_priority(next.row, next.col, vehicle->direction);
            traffic_wait_green(next.row, next.col, vehicle->direction);
            if (cell_try_occupy(vehicle->map, next.row, next.col, vehicle->id)){
                vehicle->pos = next;
                cell_release(vehicle->map, old_pos.row, old_pos.col);
                return true;
            }
        }
        else{
            srand(time(NULL));
            int turn = rand() % (2-1+1)+1;
            if (turn!=2) vehicle->direction = switch_dir(vehicle, turn);
            next = car_next_pos(vehicle);
            if (cell_try_occupy(vehicle->map, next.row, next.col, vehicle->id)){
                vehicle->pos = next;
                cell_release(vehicle->map, old_pos.row, old_pos.col);
            }
        }
        
        default: // * handling normal roads and intersection ends
        if (dir_from_tile == DIR_COUNT){
            int turn = rand() % (2-1+1)+1;
            if (turn==2){
                vehicle->direction = switch_dir(vehicle, turn);
                next = car_next_pos(vehicle);
            }
        }
        if (cell_try_occupy(vehicle->map, next.row, next.col, vehicle->id)){
            vehicle->pos = next;
            cell_release(vehicle->map, old_pos.row, old_pos.col);
            vehicle->direction = check;
            return false;
        }
        }
    return false;
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
    int global_tick = 0;
    int move_tick = 0;
    Map *map = vehicle->map;

    while (global_tick < 100){
        global_tick = clock_get_tick();
        move_tick++;
        Pos next = car_next_pos(vehicle);
        car_advance(vehicle, next, move_tick);
        clock_wait_tick(); // *waiting for clock to broadcast
    }
}