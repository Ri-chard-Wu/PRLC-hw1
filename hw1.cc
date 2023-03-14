#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <list>
#include <queue>
// #include <unordered_map>
#include <signal.h>
#include <chrono>
#include<pthread.h>
#include "tbb/concurrent_unordered_map.h"
#include "tbb/concurrent_queue.h"
using namespace std;
using namespace std::chrono;
using namespace oneapi::tbb;


#define SA_THRD_NUM 3

int n = 1000000;
int dn = 400;

#define MAP_COORD_MAX 254 // last value is reserved.
#define MAP_COORD_RSRV 255


unsigned int probe_vstdCount;
unsigned int probe_vstdClidCount;
int probe_nextStateQueue_max_size;
int probe__add_one_step__calling_count;
unsigned int probe_vstdStTbl_count;
unsigned int probe_is_reachable_vstdStTbl_count;
unsigned int probe_vstdClidStTbl_count;
unsigned int probe_is_reachable_vstdClidStTbl_count;
int thrd_cons_sa_lists_count[SA_THRD_NUM];





void probe_get_nextStateQueue_max_size(int queueSize){
    if(queueSize > probe_nextStateQueue_max_size){
        probe_nextStateQueue_max_size = queueSize;
    }
}

void probe_print_stat(){
    fprintf(stderr, "\n\n");

    fprintf(stderr, "probe_vstdCount: %d\n", probe_vstdCount);
    fprintf(stderr, "probe_vstdClidCount: %d\n", probe_vstdClidCount);
    fprintf(stderr, "probe_nextStateQueue_max_size: %d\n", probe_nextStateQueue_max_size);
    fprintf(stderr, "probe__add_one_step__calling_count: %d\n", probe__add_one_step__calling_count);
    fprintf(stderr, "probe_vstdStTbl_count: %d\n", probe_vstdStTbl_count);
    fprintf(stderr, "probe_is_reachable_vstdStTbl_count: %d\n", probe_is_reachable_vstdStTbl_count);
    fprintf(stderr, "probe_vstdClidStTbl_count: %d\n", probe_vstdClidStTbl_count);
    fprintf(stderr, "probe_is_reachable_vstdClidStTbl_count: %d\n", probe_is_reachable_vstdClidStTbl_count);
    for(int i=0;i<SA_THRD_NUM;i++){
        fprintf(stderr, "thrd_cons_sa_lists_count[%d]: %d\n", i, thrd_cons_sa_lists_count[i]);
    }
    fprintf(stderr, "\n\n");
}


void terminate(int code){
    
    fprintf(stderr, "\n\n[terminate()]: Print stat:");

    probe_print_stat();
    exit(code);
}


typedef unsigned short poskey_t;
typedef unsigned char coord_t;
typedef bitset<64> boxPos_t;
typedef bitset<64> reachablePos_t;
typedef bitset<128> state_t;



struct State{
    bitset<64> boxPos;
    unsigned char row, col;
    bitset<64> reachablePos;
};

enum Dir{
    UP,
    RIGHT,
    DOWN,
    LEFT
};

struct Action{
    unsigned char row, col;
    Dir dir;
};
struct SA{
    State state;
    Action action;
};

struct Pos{
    unsigned char row, col;
};



unsigned short pos2key(Pos pos){    
    return ((((unsigned short)pos.row) << 8) | ((unsigned short)pos.col));
}


Pos key2pos(poskey_t key){    

    Pos pos;
    poskey_t mask = 0xff;
    
    pos.col = (char)(key & mask);
    pos.row = (char)((key >> 8) & mask);

    return pos;
}


state_t state2key(State st){
    return (state_t) (st.boxPos.to_string() + st.reachablePos.to_string());
}




struct ActionNode{
    Action action;
    ActionNode *next;
};


void print_pos(Pos pos){
    fprintf(stderr, "row: %d, col: %d\n", pos.row, pos.col);
}

void print_action(Action action){
    fprintf(stderr, "row: %d, col: %d, dir: %d\n", action.row, action.col, (int)action.dir);
}


void print_action_list(list<Action> action_list){
    
    if(action_list.empty()){
        fprintf(stderr, "action_list empty\n");
        return;
    }

    fprintf(stderr, "\n");

    Action action;
    while(!action_list.empty()){

        action = action_list.front();
        action_list.pop_front();

        print_action(action);

    }

    fprintf(stderr, "\n");
}



class Map{
    public:

    Map(char* filename){
        
        read_map(filename);
    }


    void print_map(char* map){
        

        for (int i=0; i<fileLen; i++){
            fprintf(stderr, "%c", map[i]);
        }

        fprintf(stderr, "\n\n");

    }    

    void print_state(State state){

        char* renderedMap = new char[fileLen];
        render_boxPos(renderedMap, state.boxPos);

        Pos pos;
        char mapObj;

        pos.row = state.row;
        pos.col = state.col;

        safe_place_object(renderedMap, pos, 'o');

        // mapObj = get_map_object(renderedMap, pos);
        // if(mapObj == '.') set_map_object(renderedMap, pos, 'O');
        // else if(mapObj == ' ') set_map_object(renderedMap, pos, 'o');

        for (int i=0; i<fileLen; i++){
            fprintf(stderr, "%c", renderedMap[i]);
        }
        fprintf(stderr, "\n\n");

    }



    void _get_file_info(char* filename){

        FILE* ptr;
        char ch;
        int col=0;
        
    
        ptr = fopen(filename, "r");
        if (NULL == ptr) {
            fprintf(stderr, "file can't be opened \n");
            exit(-1);
        }            

        fileLineNum = 0;
        fileLen = 0;           
        spaceNum = 0; 
        colMax=0;
        ch = fgetc(ptr);

        while (ch != EOF){  

            fileLen++;
            col++;

            if(ch == '\n') {
                fileLineNum++;
                col--;
                if(col > colMax) colMax = col;
                col=0;
            }
            else if(ch != '\n' && ch != EOF && ch != '#' && ch != '@' && ch != '!') {
                spaceNum++;
            }

            fprintf(stderr, "%c", ch);
            ch = fgetc(ptr);
        }

        fprintf(stderr, "\n");
        fprintf(stderr, "fileLen: %d\n", fileLen);
        fprintf(stderr, "fileLineNum: %d\n", fileLineNum);
        fprintf(stderr, "spaceNum: %d\n", spaceNum);

        fclose(ptr);

        if((colMax-1 > MAP_COORD_MAX) || (fileLineNum-1 > MAP_COORD_MAX)){
            fprintf(stderr, "[_get_file_info]: map size larger than can be handled.\n");
            exit(-1);
        }

    }


    void _read_map(char* filename){
        FILE* ptr;
        char ch;

        map = new char[fileLen];
        // renderedMap = new char[fileLen];
        int offset = 0;


        ptr = fopen(filename, "r");
        if (NULL == ptr) {
            fprintf(stderr, "file can't be opened \n");
            exit(-1);
        }

        for(int i=0; i<fileLen;i++){
            ch = fgetc(ptr); 

                
            // if(ch == '\n'){
            //     fprintf(stderr, "\n[_read_map()] detected EOF\n\n");
            // }


            if (ch == '#' || ch == '\n' || ch == EOF || ch == ' '){
                map[offset++] = ch;
            }
            else if(ch == 'X' || ch == 'O' || ch == '.'){
                map[offset++] = '.';
            }
            else if(ch == '@' || ch == '!'){
                map[offset++] = '@';
            }            
            else{
                map[offset++] = ' ';
            }  
        }      

        fclose(ptr);
    }


    void _read_state(char* filename){
        FILE* ptr;
        char ch;


        if(spaceNum > 64){
            fprintf(stderr, "spaceNum > 64 \n");
            exit(-1); 
        }

        ptr = fopen(filename, "r");
        if (NULL == ptr) {
            fprintf(stderr, "file can't be opened \n");
            exit(-1);
        }

        state.boxPos = 0b0;
        int offset = 0;
        int row = 0, col = 0;

        // for(int i=0; i<fileLen-1;i++){
        for(int i=0; i<fileLen;i++){
            
            ch = fgetc(ptr); 

            if(ch == '\n'){
                row++; col = 0;
                continue;
            }
            
            if(ch != '#'){
                if(ch == ' ' || ch == '.'){
                    state.boxPos.reset(size_t(offset++));
                }
                else if(ch == 'x' || ch == 'X'){
                    state.boxPos.set(size_t(offset++), true);
                }
                else if(ch == 'o' || ch == 'O'){
                    state.boxPos.reset(size_t(offset++));
                    state.row = row;
                    state.col = col;
                }
                else if(ch == '!'){
                    state.row = row;
                    state.col = col;
                }                
            }

            col++;
        }      

        fclose(ptr);


    }




    void _get_row_ends(){

        rowBegins = new int[fileLineNum];


        int offset = 0;
        rowBegins[offset++] = 0;

        for (int i=1; i<fileLen-1; i++){
            if(map[i] == '\n'){
                rowBegins[offset++] = i+1;
            }                
        }  

        // fprintf(stderr, "offset: %d\n", offset);
        
        // if(map[fileLen-1] == EOF){
        //     fprintf(stderr, "\n\nis EOF\n\n");
        // }
        // else if(map[fileLen-1] == '\n'){
        //     fprintf(stderr, "\n\nis newline char\n\n");
        // }else{

        // }

    }

    void read_map(char* filename){

        _get_file_info(filename);
        _read_map(filename);
        _read_state(filename);
        _get_row_ends();


        // fprintf(stderr, "a\n");
        // // State state = map->get_state();
        // cerr<<"state.boxPos: "<<state.boxPos<<"\n";
        // fprintf(stderr, "b\n");
    }



    bool is_done(State state){
        char* renderedMap = new char[fileLen];
        render_boxPos(renderedMap, state.boxPos);
        char ch;

        for (int i=0; i<fileLen-1; i++){
            ch = renderedMap[i];
            if(ch == 'x') return false;
            if(ch == '.') return false;
            if(ch == 'O') return false;
        }
        return true;
        
    }

    

    void get_available_actions(State state, list<Action>* action_list){
        // Do BFS to find what `action`'s are available given `state`.

        char* renderedMap = new char[fileLen];
        render_boxPos(renderedMap, state.boxPos);
        
        char mapObj;

        queue<Pos> nextPosQueue;
        unordered_map<poskey_t, bool> vstdPosTbl;

        Pos curPos{.row{state.row}, .col{state.col}};
        Pos nextPos;
        Action action;
        nextPosQueue.push(curPos);
        vstdPosTbl[pos2key(curPos)] = true;

        while(!nextPosQueue.empty()){
            
            curPos = nextPosQueue.front();
            nextPosQueue.pop();

            add_local_action(renderedMap, curPos, action_list); // ok
       
            for(int dir=0; dir<=3; dir++){
                add_unvisited_nextPos<bool>(renderedMap, curPos, (Dir)dir, &vstdPosTbl, &nextPosQueue);

            }
        }
    }



    void add_local_action(char* renderedMap, Pos curPos, list<Action>* action_list){
        // - Find out what adjacent boxes can be pushed.
        // - Do nothing if no box is adjacent.

        Pos probe_pos;
        char mapObj;
        Action action;

        for(int dir=0; dir <=3; dir++){
            
            move(curPos, (Dir)dir, &probe_pos);
            mapObj = get_map_object(renderedMap, probe_pos); 

            if(mapObj == 'x' || mapObj == 'X'){
            
                move(probe_pos, (Dir)dir, &probe_pos);
                mapObj = get_map_object(renderedMap, probe_pos);      

                if(mapObj == ' ' || mapObj == '.'){
                    action.dir = (Dir)dir;
                    action.row = curPos.row;
                    action.col = curPos.col;

                    if(!is_dead_action(renderedMap, action)){

                        action_list->push_back(action);
                    }
                               
                }
            }
        }
    }



    bool is_dead_action(char* renderedMap, Action action){

        Pos curPos{.row{action.row}, .col{action.col}};
        Pos o_pos, x_pos; // next pos
        
        move(curPos, action.dir, &o_pos);
        move(o_pos, action.dir, &x_pos);

        char* nextRenderedMap = new char[fileLen];
        copy_map(renderedMap, nextRenderedMap);

        // renderedMap has no 'o'.
        safe_place_object(nextRenderedMap, o_pos, ' '); // remove 'x'.


        if(is_dead_corner(nextRenderedMap, x_pos, action.dir)){
            return true;
        }
        else if(is_dead_wall(nextRenderedMap, x_pos, action.dir)){
            return true;
        }

        return false;         
    }

    bool is_dead_wall(char* renderedMap, Pos x_pos, Dir pushDir){

        // renderedMap has no 'o', and has the target 'x' removed.
        int trgtTileCnt, xCnt;
        int adjDir1, adjDir2;
        Pos probePos, wallProbePos;
        bool isNotBounded;
        char mapObj;


        move(x_pos, pushDir, &probePos);
        mapObj = get_map_object(renderedMap, probePos);
        if(mapObj != '#') return false; // not even a wall.


        adjDir1 = (((int)pushDir)+1)%4;
        adjDir2 = (((int)pushDir)+3)%4;

        isNotBounded = false;
        trgtTileCnt = 0;
        mapObj = get_map_object(renderedMap, x_pos);
        if(mapObj == '.')trgtTileCnt++;
        xCnt = 1;        

        // check two adjacent sides. If one side is not bounded, then not dead.
        for(int adjDir=0; adjDir<=4; adjDir++){
            if(adjDir != adjDir1 && adjDir != adjDir2) continue;

            move(x_pos, (Dir)adjDir, &probePos); // initialize position.
            mapObj = get_map_object(renderedMap, probePos);  

            while(mapObj != '#'){

                if(mapObj == 'x' || mapObj == 'X'){
                    xCnt++;                 
                } 

                if(mapObj == 'X' || mapObj == '.') {
                    trgtTileCnt++;
                }
                

                move(probePos, pushDir, &wallProbePos);
                mapObj = get_map_object(renderedMap, wallProbePos);
                if(mapObj == ' ' || mapObj == '.'){
                    isNotBounded = true;
                    break;
                } 

                move(probePos, (Dir)adjDir, &probePos);
                mapObj = get_map_object(renderedMap, probePos);
            }

            if(isNotBounded) break; // early stop.
        }

        if(isNotBounded){
            return false;
        }
        else{
            // if(trgtTileCnt < xCnt){
            //     fprintf(stderr, "\nxCnt:%d, trgtTileCnt:%d\n", xCnt, trgtTileCnt); 
            // }
            return trgtTileCnt < xCnt;
        }
    }


    void copy_map(char* fromMap, char* toMap){
        for(int i=0; i<fileLen; i++){
            toMap[i] = fromMap[i];
        }
    }

    bool is_not_space(char ch){
        return ch == '#' || ch != '@';
    }

    bool is_dead_corner(char* renderedMap, Pos x_pos, Dir pushDir){
        // renderedMap has no 'o', and has the target 'x' removed.

        // Fast path
        // - a box pushed into corner (2 '#' or 1 '#' + 1 'x') and is not on '.' 

        int dir1, dir2, adjDir1, adjDir;
        Pos probe_pos, probe_pos1, probe_pos2;
        char mapObj, mapObj_adj1, mapObj_adj2, mapObj_x_pos;
        bool is_dead;

        mapObj_x_pos = get_map_object(renderedMap, x_pos);


        for(int dir=((int)pushDir); dir <= ((int)pushDir) + 1; dir++){
            dir1 = dir % 4; 
            dir2 = (dir1 + 3)%4;  


            move(x_pos, (Dir)dir1, &probe_pos1);
            mapObj_adj1 = get_map_object(renderedMap, probe_pos1);
            if(mapObj_adj1 == ' ' || mapObj_adj1 == '.' ){continue;}

            move(x_pos, (Dir)dir2, &probe_pos2);
            mapObj_adj2 = get_map_object(renderedMap, probe_pos2);
            if(mapObj_adj2 == ' ' || mapObj_adj2 == '.' ){continue;}


            if(mapObj_adj1 == '#' && mapObj_adj2 == '#' && mapObj_x_pos != '.'){ 
                return true;
            }
            else if((mapObj_adj1 == '#' && mapObj_adj2 == 'x') ||
                    (mapObj_adj1 == '#' && mapObj_adj2 == 'X' && mapObj_x_pos == ' ')){
                
                

                move(probe_pos2, (Dir)(dir1%4), &probe_pos);
                mapObj = get_map_object(renderedMap, probe_pos);
                if(mapObj != ' ' && mapObj != '.')return true;


            }                   
            else if((mapObj_adj1 == 'x' && mapObj_adj2 == '#' )||
                    (mapObj_adj1 == 'X' && mapObj_adj2 == '#' && mapObj_x_pos == ' ')){


                move(probe_pos1, (Dir)(dir2%4), &probe_pos);
                mapObj = get_map_object(renderedMap, probe_pos);
                if(mapObj != ' ' && mapObj != '.')return true;

          

            }
            else if((mapObj_adj1 == 'x' && mapObj_adj2 == 'x') ||
                    (mapObj_adj1 == 'x' && mapObj_adj2 == 'X') ||
                    (mapObj_adj1 == 'X' && mapObj_adj2 == 'x') ||
                    (mapObj_adj1 == 'X' && mapObj_adj2 == 'X' && mapObj_x_pos == ' ')){


                move(probe_pos1, (Dir)(dir2%4), &probe_pos);
                mapObj = get_map_object(renderedMap, probe_pos);
                if(mapObj != ' ' && mapObj != '.')return true;
                  
            }              
        }     

        return false;

    }
    



    template<class dictValue_t>
    bool is_pos_visited(unordered_map<poskey_t, dictValue_t> *vstdPosTbl, Pos pos){
        return !(vstdPosTbl->find(pos2key(pos)) == vstdPosTbl->end());
    }




    template<class dictValue_t>
    bool add_unvisited_nextPos(char *renderedMap, Pos curPos, Dir dir, 
                unordered_map<poskey_t, dictValue_t> *vstdPosTbl, queue<Pos> *nextPosQueue){
  
        Pos nextPos;
        char mapObj;

        move(curPos, dir, &nextPos);
        mapObj = get_map_object(renderedMap, nextPos); 

        if(mapObj == '.' || mapObj == ' ' || mapObj == '@'){
            if(!is_pos_visited<dictValue_t>(vstdPosTbl, nextPos)){
        
                nextPosQueue->push(nextPos);
                (*vstdPosTbl)[pos2key(nextPos)] = true;
                return true;
            }else{
                return false;                    
            }
        }

        return false;
    }




    void move(Pos curPos, Dir dir, Pos *nextPos){

        nextPos->row = curPos.row;
        nextPos->col = curPos.col;

        if(dir == UP) nextPos->row -= 1;
        else if(dir == RIGHT) nextPos->col += 1;
        else if(dir == DOWN) nextPos->row += 1;
        else if(dir == LEFT) nextPos->col -= 1;
    }


    State get_state(){
        return state;
    }

    char get_map_object(char *renderedMap, Pos pos){
        return renderedMap[rowBegins[pos.row] + pos.col];
    }


    void find_reachable_pos(char* renderedMap, Pos o_pos, State* state){

     
        // fprintf(stderr, "------------------------------------------------\n");
        // fprintf(stderr, "[find_reachable_pos()] debug.  before set 'r', renderedMap: \n");
        // print_map(renderedMap);






        char mapObj;

        queue<Pos> nextPosQueue;
        unordered_map<poskey_t, bool> vstdPosTbl;
        

        Pos probe_pos, curPos;
        curPos.row = o_pos.row;
        curPos.col = o_pos.col;
        
        // safe_place_object(renderedMap, curPos, 'o');
        nextPosQueue.push(curPos);
        vstdPosTbl[pos2key(curPos)] = true;
    

        while(!nextPosQueue.empty()){

    
            curPos = nextPosQueue.front();
            nextPosQueue.pop();

            for(int dir=0; dir<=3; dir++){
            

                move(curPos, (Dir)dir, &probe_pos);
                mapObj = get_map_object(renderedMap, probe_pos); 


                if(mapObj == '.' || mapObj == ' ' || mapObj == '@'){

                    if(!is_pos_visited<bool>(&vstdPosTbl, probe_pos)){
                        nextPosQueue.push(probe_pos);
                        vstdPosTbl[pos2key(probe_pos)] = true;

                        if(mapObj != '@'){set_map_object(renderedMap, probe_pos, 'r');}
                        else{set_map_object(renderedMap, probe_pos, 'R');}
                    }
                }
            }
        }

        // fprintf(stderr, "[find_reachable_pos()] debug.  after set 'r', renderedMap: \n");
        // print_map(renderedMap);


        map2state(renderedMap, state);


        




        // fprintf(stderr, "[find_reachable_pos()] debug. re-render: \n");

        // char* debugMap = new char[fileLen];

        // render_boxPos(debugMap, state->boxPos);
        // Pos debugPos;
        // debugPos.row = state->row;
        // debugPos.col = state->col;
        // safe_place_object(debugMap, debugPos, 'o');
        // print_map(debugMap);

        // render_reachablePos(debugMap, state->reachablePos);
        // print_map(debugMap);

        // // exit(1);
        // static int count = 0;
        // if(count++ > 50){
        //     exit(1);
        // }
        
    }

    void render_reachablePos(char *renderedMap, reachablePos_t reachablePos){

        char ch;
        int offset = 0;

        for (int i=0; i<fileLen-1; i++){

            ch = map[i];
            
            if(ch == ' ' || ch == '.'){
                if(reachablePos[offset++] == 1) ch = 'r';
            }
            else if(ch == '@'){
                if(reachablePos[offset++] == 1) ch = 'R';
            }
            
            renderedMap[i] = ch;
        }
    }

    void render_boxPos(char *renderedMap, boxPos_t boxPos){

        char ch;
        int offset = 0;

        for (int i=0; i<fileLen-1; i++){

            ch = map[i];
            
            if(ch == ' '){
                if(boxPos[offset++] == 1) ch = 'x';
            }
            else if(ch == '.'){
                if(boxPos[offset++] == 1) ch = 'X';
            }
            
            renderedMap[i] = ch;
        }
    }


    void reset_renderedMap(char* renderedMap){
        for (int i=0; i<fileLen-1; i++){
            renderedMap[i] = map[i];
        }
    }


    void act(State cur_state, Action action, State* next_state){
        // - Push box, need update position of one 'x' and 'o'.

        char* renderedMap = new char[fileLen];
        render_boxPos(renderedMap, cur_state.boxPos);  


        char mapObj;
        Pos o_pos, x_pos;
        
        o_pos.row = action.row;
        o_pos.col = action.col;
        move(o_pos, action.dir, &o_pos); // x_pos: current position of 'x'

        // replace 'x' in old position by 'o'.
        mapObj = get_map_object(renderedMap, o_pos); 
        if(mapObj == 'x') set_map_object(renderedMap, o_pos, 'o');
        else if(mapObj == 'X') set_map_object(renderedMap, o_pos, 'O');
        else {fprintf(stderr, "[act()] mapObj != 'x' \n"); exit(-1);}

        // place 'x' on its new position. 
        move(o_pos, action.dir, &x_pos);
        mapObj = get_map_object(renderedMap, x_pos); 
        if(mapObj == ' ') set_map_object(renderedMap, x_pos, 'x');
        else if(mapObj == '.') set_map_object(renderedMap, x_pos, 'X');
        else {fprintf(stderr, "[act()] `x` new position not a space. \n"); exit(-1);}

        // if(probe_vstdCount > 0){
        //     fprintf(stderr, "[act()] after set map. \n");
        //     print_map(renderedMap);

        // }

        // map2state(renderedMap, next_state);
        find_reachable_pos(renderedMap, o_pos, next_state);


    }



    void safe_place_object(char* map, Pos pos, char ch){
        char mapObj;
        mapObj = get_map_object(map, pos);

        if(ch == 'o' || ch == 'O'){
            if(mapObj == ' ') set_map_object(map, pos, 'o');
            else if(mapObj == '.') set_map_object(map, pos, 'O');
            else if(mapObj == '@') set_map_object(map, pos, '!');
            else{fprintf(stderr, "\n[safe_place_object()]:invalid placement.\n\n"); exit(-1);}
        }
        else if(ch == 'x' || ch == 'X'){
            if(mapObj == ' ') set_map_object(map, pos, 'x');
            else if(mapObj == '.') set_map_object(map, pos, 'X');
            else{fprintf(stderr, "\n[safe_place_object()]:invalid placement.\n\n"); exit(-1);}
        }
        else if(ch == ' '){
            if(mapObj == 'o' || mapObj == 'x') set_map_object(map, pos, ' ');
            else if(mapObj == 'O' || mapObj == 'X') set_map_object(map, pos, '.');
        }
        else{
            set_map_object(map, pos, ch);
        }        
    }



    void set_map_object(char* map, Pos pos, char ch){
        map[rowBegins[pos.row] + pos.col] = ch;
    }



    void map2state(char* renderedMap, State* state){
        char ch;

        state->boxPos = 0b0;
        state->reachablePos = 0b0;
        int ofst_x=0, ofst_o = 0;
        int row = 0, col = 0;

        for(int i=0; i<fileLen-1;i++){
            
            ch = renderedMap[i]; 

            // fprintf(stderr, "\n[map2state()] ofst_o: %d, ofst_x: %d\n", ofst_o, ofst_x);

            if(ch == '\n'){ row++; col = 0; continue; }

            if(ch != '#'){ 

                if(ch == ' ' || ch == '.'){
                    ofst_x++;
                    ofst_o++;
                }   
                else if(ch == 'R'){
                    state->reachablePos.set(size_t(ofst_o++), true);
                }
                else if(ch == 'r'){
                    ofst_x++;
                    state->reachablePos.set(size_t(ofst_o++), true);
                }                             
                else if(ch == 'x' || ch == 'X'){
                    ofst_o++;
                    state->boxPos.set(size_t(ofst_x++), true);
                }
                else if(ch == 'o' || ch == 'O'){
                    ofst_x++;
                    state->reachablePos.set(size_t(ofst_o++), true);
                    state->row = row;
                    state->col = col;
                }
                else if(ch == '!'){
                    state->reachablePos.set(size_t(ofst_o++), true);
                    state->row = row;
                    state->col = col;                    
                }


            }             

            col++;
        }      
    }
    

    // void map2state(char* renderedMap, State* state){
    //     char ch;

    //     state->boxPos = 0b0;
    //     int offset = 0;
    //     int row = 0, col = 0;

    //     for(int i=0; i<fileLen-1;i++){
            
    //         ch = renderedMap[i]; 

    //         if(ch == '\n'){ row++; col = 0; continue; }

    //         if(ch != '#'){ 

    //             if(ch == ' ' || ch == '.'){
    //                 offset++;
    //             }
    //             else if(ch == 'x' || ch == 'X'){
    //                 state->boxPos.set(size_t(offset++), true);
    //             }
    //             else if(ch == 'o' || ch == 'O'){
    //                 offset++;
    //                 state->row = row;
    //                 state->col = col;
    //             }
    //             else if(ch == '!'){
    //                 state->row = row;
    //                 state->col = col;                    
    //             }
    //         }             

    //         col++;
    //     }      
    // }


    bool is_reachable(State state, Pos pos1, Pos pos2){
        

        char* renderedMap = new char[fileLen];
        render_boxPos(renderedMap, state.boxPos);  
        char mapObj;

        queue<Pos> nextPosQueue;
        unordered_map<poskey_t, bool> vstdPosTbl;
        
        Pos probe_pos, curPos = pos1;
        nextPosQueue.push(curPos);
        vstdPosTbl[pos2key(curPos)] = true;
                        
        while(!nextPosQueue.empty()){
            curPos = nextPosQueue.front();
            nextPosQueue.pop();
            // vstdPosTbl[pos2key(curPos)] = true;

            for(int dir=0; dir<=3; dir++){
                add_unvisited_nextPos<bool>(renderedMap, curPos, (Dir)dir, &vstdPosTbl, &nextPosQueue);
            }


            if(is_pos_visited<bool>(&vstdPosTbl, pos2)){return true;}
        }

        return false;

    }


    void generate_rvrsState(State curState, Action curAction, State* rvrsState){
        
        // fprintf(stderr, "==========================\n[generate_rvrsState()] print curState:\n\n");
        // print_state(curState);

        char* renderedMap = new char[fileLen];
        render_boxPos(renderedMap, curState.boxPos); 

        Pos ocurPos, xcurPos, oprevPos, xprevPos;
        
        int dir = (int)curAction.dir;
        int opstDir = (dir + 2)%4; 

        ocurPos.row = curState.row;
        ocurPos.col = curState.col;

        move(ocurPos, (Dir)opstDir, &oprevPos);
        move(ocurPos, (Dir)dir, &xcurPos);
        xprevPos = ocurPos;
        
        // fprintf(stderr, "\n[generate_rvrsState()] call safe place obj:\n\n");

        safe_place_object(renderedMap, xcurPos, ' ');
        safe_place_object(renderedMap, xprevPos, 'x');
        safe_place_object(renderedMap, oprevPos, 'o');


        // map2state(renderedMap, rvrsState);

        find_reachable_pos(renderedMap, oprevPos, rvrsState);



        // fprintf(stderr, "\n[generate_rvrsState()] print reversed state:\n\n");
        // print_state(*rvrsState);

        // print_action(curAction);
        // fprintf(stderr, "==========================\n");

    }


    void move_by_action(Action action, Pos* pos){

        Pos curPos;
        curPos.row = action.row;
        curPos.col = action.col;
        move(curPos, action.dir, pos);      

    }



    bool get_steps(boxPos_t boxPos, Pos fromPos, Pos toPos, list<Dir>* steps){
        
        // fprintf(stderr, "\n[get_steps()] fromPos, toPos:\n");
        // print_pos(fromPos);
        // print_pos(toPos);


        char* renderedMap = new char[fileLen];
        render_boxPos(renderedMap, boxPos);

        queue<Pos> nextPosQueue;
        unordered_map<poskey_t, poskey_t> vstdPosTbl;

        Pos curPos = fromPos, nextPos;
        poskey_t curPosKey;
        Action action;

        nextPosQueue.push(curPos);
        vstdPosTbl[pos2key(curPos)] = (MAP_COORD_RSRV << 8) | MAP_COORD_RSRV;
        
        while(!nextPosQueue.empty()){
            
            curPos = nextPosQueue.front();
            nextPosQueue.pop();
            curPosKey = pos2key(curPos);

            for(int dir=0; dir<=3; dir++){

                if(add_unvisited_nextPos<poskey_t>(renderedMap, curPos, 
                                            (Dir)dir, &vstdPosTbl, &nextPosQueue)){
                    
                    nextPos = nextPosQueue.back();
                    vstdPosTbl[pos2key(nextPos)] = curPosKey;
                }
            }
        }


        if(is_pos_visited<poskey_t>(&vstdPosTbl, toPos)){
            
            Pos curPos, prevPos;
            Dir step;
            poskey_t prevPoskey, curPosKey, nullPosKey = ((MAP_COORD_RSRV << 8) | MAP_COORD_RSRV);
            

            curPos = toPos;
            curPosKey = pos2key(curPos);
            prevPoskey = vstdPosTbl[curPosKey];

            while(prevPoskey != nullPosKey){
                prevPos = key2pos(prevPoskey);

                infer_local_dir(prevPos, curPos, &step);
                steps->push_front(step);

                curPos = prevPos;
                curPosKey = prevPoskey;
                prevPoskey = vstdPosTbl[curPosKey];
            }

            // fprintf(stderr, "\n[get_steps()] steps.size(): %d\n", steps->size());
            // exit(1);
            
            return true;
        }
        else{
            return false;
        }


    }


    void infer_local_dir(Pos fPos, Pos tPos, Dir* dir){

        coord_t frow, fcol, trow, tcol;

        frow = fPos.row;
        fcol = fPos.col;

        trow = tPos.row;
        tcol = tPos.col;
        

        // fprintf(stderr, "\n[infer_local_dir()] frow: %d, fcol: %d\n", frow, fcol);
        // fprintf(stderr, "\n[infer_local_dir()] trow: %d, tcol: %d\n", trow, tcol);



        if(frow == trow){
            if(fcol - tcol == 1){
                *dir = LEFT;
            }
            else if(tcol - fcol == 1){
                *dir = RIGHT;
            }
            else{

                fprintf(stderr, "\n[infer_local_dir()] Invalid input, not adjacent col's:\n\n");
                exit(-1);                    
            }
        }
        else if(fcol == tcol){
            if(frow - trow == 1){
                *dir = UP;
            }
            else if(trow - frow == 1){
                *dir = DOWN;
            }
            else{
                fprintf(stderr, "\n[infer_local_dir()] Invalid input, not adjacent row's:\n\n");
                exit(-1);                    
            }
        }
        else{
            fprintf(stderr, "\n[infer_local_dir()] Invalid input, pos's not on same row and col:\n\n");
            exit(-1);
        }
    }


    char *map;

    int *rowEnds;
    int *rowBegins;

    int fileLen;
    int fileLineNum;
    int spaceNum;
    int colMax;

    State state;    
};



class Solver{
    public:

    Solver(char* filename){
        map = new Map(filename);
        map->print_map(map->map);
        
        done = false;
        start_thread();
        thrd_cons_nxStQueue(0);
        join_thread();
        
        recover_steps();
        output_steps();

        probe_print_stat();
        
    }




    void start_thread()
    {
        int ret;
        entryArg* argPtr;


        saThrdNum = 5;
        
       
        sa_lists = new concurrent_queue<SA>[saThrdNum];
        saThrds = new pthread_t[saThrdNum];
        for(int i=0;i<saThrdNum;i++){
            argPtr = new entryArg;
            argPtr->objPtr = this;
            argPtr->threadId = i;

            ret = pthread_create(&saThrds[i], NULL, thrd_cons_sa_lists_entry, (void *)argPtr);
            if(ret != 0) printf("create thread failed.\n");
        }
    }


    void join_thread()
    {

        for (int i = 0; i < saThrdNum; i++){
            pthread_join(saThrds[i], NULL);
        }  
 
    }



    static void * thrd_cons_nxStQueue_entry(void *arg) {
        entryArg *argPtr = (entryArg *)arg;

        int threadId = argPtr->threadId;
        Solver *objPtr = (Solver *)argPtr->objPtr;
        objPtr->thrd_cons_nxStQueue(threadId);
        
        return NULL;
    }


    static void * thrd_cons_sa_lists_entry(void *arg) {
        entryArg *argPtr = (entryArg *)arg;

        int threadId = argPtr->threadId;
        Solver *objPtr = (Solver *)argPtr->objPtr;
        objPtr->thrd_cons_sa_lists(threadId);
        
        return NULL;
    }




    void thrd_cons_nxStQueue(int threadId){

        list<Action> action_list;
        State state;
        SA sa;
        bool has_new_nextState_data;

        while(!done){

            has_new_nextState_data = false;

            if(!nextStateQueue.empty()){
                if(!nextStateQueue.try_pop(state)){
                    fprintf(stderr, "\n[thrd_cons_nxStQueue()] pop failed.\n\n"); 
                }
                has_new_nextState_data = true;
            }
            
        
            if(has_new_nextState_data){
                map->get_available_actions(state, &action_list);
                thrd_sa_distributer(state, &action_list);
            }
        }
    }




    void thrd_sa_distributer(State state, list<Action>* action_list){

        int minLenThrdId, minLen, sa_list_size;
        Action curAction;
        SA sa;

        list<SA> sa_list;

        while(!action_list->empty()){
            curAction = action_list->front();
            action_list->pop_front();            
            sa.action = curAction;
            sa.state = state;
            sa_list.push_back(sa);
        }


        while(!sa_list.empty()){

            sa = sa_list.front();
            sa_list.pop_front();

            minLenThrdId = 0;
            minLen = 10000;            
            for(int i=0; i < saThrdNum; i++){
                sa_list_size = sa_lists[i].unsafe_size();
                if(minLen > sa_list_size){
                    minLen = sa_list_size;
                    minLenThrdId = i;
                }
            }
            sa_lists[minLenThrdId].push(sa);
        }
    }







    void thrd_cons_sa_lists(int threadId){

        bool has_new_sa_data;
        Action curAction;
        State nextSt, curSt;
        SA sa;

        if(threadId == 0){
            curAction.row = MAP_COORD_RSRV;
            curAction.col = MAP_COORD_RSRV;
            
            State initSt = map->get_state();
            Pos o_pos;
            o_pos.row = initSt.row;
            o_pos.col = initSt.col;

            char* renderedMap = new char[map->fileLen];
            map->render_boxPos(renderedMap, initSt.boxPos);
            map->safe_place_object(renderedMap, o_pos, 'o');
            map->find_reachable_pos(renderedMap, o_pos, &initSt);

            nextStateQueue.push(initSt);

            vstdStTbl[state2key(initSt)] = curAction;            
        }


        while(!done){ 
            
            has_new_sa_data = false;
            if(!sa_lists[threadId].empty()){
                
                if(!sa_lists[threadId].try_pop(sa)){fprintf(stderr, "\npop failed.\n\n");}

                curSt = sa.state;
                curAction = sa.action;

                has_new_sa_data = true;
            }
            


            if(has_new_sa_data){

  

                map->act(curSt, curAction, &nextSt); 

 
            
                if(map->is_done(nextSt)){

                   

                    doneState = nextSt;
                    doneAction = curAction;
                    done = true;

                    if(threadId == 0)fprintf(stderr, "\ndone!\n\n"); 
                    map->print_state(doneState);   
                    return;      
                } 

                

                if(!is_in_vstdStTbl(&vstdStTbl, state2key(nextSt))){
                    
           

                    nextStateQueue.push(nextSt);
                    vstdStTbl[state2key(nextSt)] = curAction;
                    
                }

    

            }
        }
    }



    // void add_visited_state(concurrent_queue<State>* nextStateQueue , 
    //             concurrent_unordered_map<state_t, Action> *vstdStTbl, 
    //                 State state, Action action){

    //     if(!is_in_vstdStTbl(vstdStTbl, state2key(state))){
    //         nextStateQueue->push(state);
    //         (*vstdStTbl)[state2key(state)] = action;
    //         return;
    //     }
    // }



    // void insert_ActionNode(concurrent_unordered_map<bitset<64>, ActionNode*> *vstdClidStTbl, State state, Action action){

    //     ActionNode *cur = new ActionNode;
    //     cur->action = action;

    //     if(!is_in_vstdClidStTbl(vstdClidStTbl, state.boxPos)){
    //         cur->next = NULL;
    //     }
    //     else{
    //         cur->next = (*vstdClidStTbl)[state.boxPos];
    //     }

    //     probe_vstdClidCount++;
    //     (*vstdClidStTbl)[state.boxPos] = cur;

    // }




    bool is_in_vstdStTbl(concurrent_unordered_map<state_t, Action> *vstdStTbl, state_t stateKey){
        bool ret = !(vstdStTbl->find(stateKey) == vstdStTbl->end()); 
        return ret;
    }



    void output_steps(){
        for(int i = stepsOfst - 1; i >= 0; i--){
            printf("%c", stepsBuf[i]);
        }
        printf("\n");
    }

    void recover_steps(){

        stepsBufSize = 1024;
        stepsBuf = new char[stepsBufSize];
        stepsOfst = 0;

        State prevState, curState, rvrsState;
        Action prevAction, curAction;
        int dir, opstDir, retCode;


        curState = doneState;
        curAction = doneAction;
        add_one_step(curAction.dir, true);


        generate_rvrsState(curState, curAction, &rvrsState); // get prev boxPos and player pos.

        fprintf(stderr, "\n[recover_steps()] debug:\n\n");
        map->print_state(curState);
        map->print_state(rvrsState);

        // char* debugMap = new char[map->fileLen];
        // map->render_reachablePos(debugMap, rvrsState.reachablePos);
        // map->print_map(debugMap);
        // exit(1);

        retCode = find_vstd_prevStateAction(rvrsState, &prevState, &prevAction);
        
        // fprintf(stderr, "\n[recover_steps()] retCode: %d\n\n", retCode);

        while(retCode == 0){ // 0 -> both `prevState` and `prevAction found.
            

            // fprintf(stderr, "\n[recover_steps()] debug:\n\n");
            // map->print_state(curState);
            // map->print_state(rvrsState);
            
            add_intermediate_steps(prevState, rvrsState);

            curState = prevState;
            curAction = prevAction;
            add_one_step(curAction.dir, true);
            generate_rvrsState(curState, curAction, &rvrsState); // get prev boxPos and player pos.
            retCode = find_vstd_prevStateAction(rvrsState, &prevState, &prevAction);
            // fprintf(stderr, "\n[recover_steps()] retCode: %d\n\n", retCode);
        }

        // fprintf(stderr, "\n[recover_steps()] debug:\n\n");
        // map->print_state(curState);
        // map->print_state(rvrsState);

        if(retCode == 2){ // 2 -> only `prevState`, which is true for initial state
            add_intermediate_steps(prevState, rvrsState);
        }
        else{
            fprintf(stderr, "\n[recover_steps()] breakout from loop before init state found. \n\n");
            terminate(-1);
        }
        

    }


    void test_get_steps(boxPos_t boxPos, Pos fromPos, Pos toPos, list<Dir> steps){

        fprintf(stderr, "\n[test_get_steps()]: print map & steps: \n\n");
        char* debugMap = new char[map->fileLen];
        map->render_boxPos(debugMap, boxPos); 

        if(!((fromPos.row == toPos.row)&&(fromPos.col == toPos.col))){

            map->safe_place_object(debugMap, fromPos, 'o');
            map->safe_place_object(debugMap, toPos, 'o');
            map->print_map(debugMap);

        }

        fprintf(stderr, "\n");

        while (!steps.empty()){
            fprintf(stderr, "%d ", (int)steps.front());
            steps.pop_front();
        }

        fprintf(stderr, "\n\n");
    }



    void add_intermediate_steps(State prevState, State rvrsState){
        
        Pos fromPos{.row{prevState.row}, .col{prevState.col}};
        Pos toPos{.row{rvrsState.row}, .col{rvrsState.col}};

        // fprintf(stderr, "\n[add_intermediate_steps()] print pos & steps:\n\n");
        // print_pos(fromPos);
        // print_pos(toPos);

        list<Dir> steps;

        map->get_steps(prevState.boxPos, fromPos, toPos, &steps);
        
        // test_get_steps(prevState.boxPos, fromPos, toPos, steps);

        while (!steps.empty()){

            // fprintf(stderr, "%d ", (int)steps.back());
            add_one_step(steps.back(), false);
            steps.pop_back();
        }

    }





    int find_vstd_prevStateAction(State rvrsState, State* prevState, Action* prevAction){
        // - Three situations:
        // - 1. both `prevState` and `prevAction found -> code == 0
        // - 2. both `prevState` and `prevAction not found -> code == 1
        // - 3. `prevAction` not found -> code == 2 (only for initial state)


        // - Not in vstdStTbl.
        if(!is_in_vstdStTbl(&vstdStTbl, state2key(rvrsState)))
        {
            return 1;
        }


        // - In vstdStTbl.
        Pos rvrsPos, prevPos, probePos;
        Action  probeAction;
        bool is_initAction = false;


        // get probePos
        probeAction = vstdStTbl[state2key(rvrsState)];

        // Terminating condition. Initial state has no action. Get init pos of player from map.
        if((probeAction.row == MAP_COORD_RSRV) && (probeAction.row == MAP_COORD_RSRV)){
            probePos.row = map->state.row;
            probePos.col = map->state.col;
            is_initAction = true;
        }
        else{
            map->move_by_action(probeAction, &probePos);
        }


        prevState->boxPos = rvrsState.boxPos;
        prevState->row = probePos.row;
        prevState->col = probePos.col;

        *prevAction = probeAction;
        if(is_initAction) return 2;
        else return 0;

    }


    void generate_rvrsState(State curState, Action curAction, State* rvrsState){
        map->generate_rvrsState(curState, curAction, rvrsState);
    }


    void add_one_step(Dir dir, bool cap){

        cap = true;

        probe__add_one_step__calling_count++;

        if(stepsOfst >= stepsBufSize){
            fprintf(stderr, "\n[add_one_step()] stepsBuf[] overflowed:\n\n");
            terminate(-1);
        }

        if(dir == UP) stepsBuf[stepsOfst] = cap?'W':'w';
        else if(dir == RIGHT) stepsBuf[stepsOfst] = cap?'D':'d';
        else if(dir == DOWN) stepsBuf[stepsOfst] = cap?'S':'s';
        else if(dir == LEFT) stepsBuf[stepsOfst] = cap?'A':'a';
        
        stepsOfst++;
    }
    

    struct entryArg{
        Solver* objPtr;
        int threadId;
    };



    // list<SA>* sa_lists;
    concurrent_queue<SA>* sa_lists;
    
    int saThrdNum;
    int nxStThrdNum;

    pthread_t* nxStThrds;
    pthread_t* saThrds;




    int stepsBufSize;
    char* stepsBuf;
    int stepsOfst;

    concurrent_unordered_map<state_t, Action> vstdStTbl;
    concurrent_queue<State> nextStateQueue;
    

    Map* map;

    bool done;
    State doneState;
    Action doneAction;    
};




void signal_callback_handler(int signum) {
    
    fprintf(stderr, "\n\nTerminate program. Print stat:");

    probe_print_stat();

    exit(signum);
}


int main(int argc, char** argv) {

    signal(SIGINT, signal_callback_handler);

    fprintf(stderr, "argv[1]: %s\n", argv[1]);

    probe_vstdCount = 0;
    probe_vstdClidCount = 0;
    probe_nextStateQueue_max_size = 0;
    probe__add_one_step__calling_count = 0;
    probe_vstdStTbl_count = 0;
    probe_is_reachable_vstdStTbl_count = 0;
    probe_vstdClidStTbl_count = 0;
    probe_is_reachable_vstdClidStTbl_count = 0;
    for(int i=0;i<SA_THRD_NUM;i++){
        thrd_cons_sa_lists_count[i] = 0;
    }
    Solver solver(argv[1]);
    

    return 0;
}