#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <bitset>
#include <list>
#include <queue>
#include <unordered_map>
#include <signal.h>
using namespace std;




#define MAP_COORD_MAX 254 // last value is reserved.
#define MAP_COORD_RSRV 255


unsigned int probe_vstdCount;
unsigned int probe_vstdClidCount;
int probe_nextStateQueue_max_size;

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
    
    fprintf(stderr, "\n\n");
}


struct State{
    bitset<64> boxPos;
    unsigned char row, col;
};


struct Pos{
    unsigned char row, col;
};


typedef unsigned short poskey_t;
typedef unsigned char coord_t;


unsigned short pos2key(Pos pos){    
    return ((((unsigned short)pos.row) << 8) | ((unsigned short)pos.col));
}


Pos key2pos(poskey_t key){    

    Pos pos;
    poskey_t mask = 0xff;
    
    pos.col = key & mask;
    pos.row = (key >> 8) & mask;

    return pos;
}


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

struct ActionNode{
    Action action;
    ActionNode *next;
};


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

            mapObj = get_map_object(renderedMap, pos);
            if(mapObj == '.') set_map_object(renderedMap, pos, 'O');
            else if(mapObj == ' ') set_map_object(renderedMap, pos, 'o');

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
                else if(ch != '\n' && ch != EOF && ch != '#') {
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
                if (ch == '#' || ch == '\n' || ch == EOF || ch == ' '){
                    map[offset++] = ch;
                }
                else if(ch == 'X' || ch == 'O' || ch == '.'){
                    map[offset++] = '.';
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

            for(int i=0; i<fileLen-1;i++){
                
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
                }

                col++;
            }      

            fclose(ptr);
        }

        void _get_row_ends(){

            rowBegins = new int[fileLineNum];


            int offset = 0;
            rowBegins[offset++] = 0;

            for (int i=1; i<fileLen; i++){
                if(map[i] == '\n'){
                    rowBegins[offset++] = i+1;
                }                
            }  

        }

        void read_map(char* filename){

            _get_file_info(filename);
            _read_map(filename);
            _read_state(filename);
            _get_row_ends();
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

            // fprintf(stderr, "\n[get_available_actions()] initial map to find available action:\n\n"); 
            // print_state(state);

            queue<Pos> nextPosQueue;
            unordered_map<poskey_t, bool> vstdPosTbl;

            Pos curPos{.row{state.row}, .col{state.col}};
            Pos nextPos;
            Action action;
            nextPosQueue.push(curPos);

            while(!nextPosQueue.empty()){
                
                curPos = nextPosQueue.front();
                nextPosQueue.pop();
                vstdPosTbl[pos2key(curPos)] = true;

                add_local_action(renderedMap, curPos, action_list); // ok

                // fprintf(stderr, "\n[get_available_actions()] print local action:\n\n"); 
                // print_action_list(*action_list);

                for(int dir=0; dir<=3; dir++){
                    add_unvisited_nextPos(renderedMap, curPos, (Dir)dir, &vstdPosTbl, &nextPosQueue);
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

                // fprintf(stderr, "\n[add_local_action()] mapObj: %c\n\n", mapObj);

                if(mapObj == 'x' || mapObj == 'X'){
                
                    move(probe_pos, (Dir)dir, &probe_pos);
                    mapObj = get_map_object(renderedMap, probe_pos);      

                    if(mapObj == ' ' || mapObj == '.'){
                        action.dir = (Dir)dir;
                        action.row = curPos.row;
                        action.col = curPos.col;

                        if(!is_dead_action(renderedMap, action)){

                            // fprintf(stderr, "\n[add_local_action()] is not dead action.\n\n"); 
                            action_list->push_back(action);

                        }
                        else{
                            // fprintf(stderr, "\n[add_local_action()] is dead action. dir: %d\n\n", (int)dir); 
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



            // fprintf(stderr, "\n[is_dead_action()] after render boxPos:\n\n");
            // print_map(nextRenderedMap);    

            // fprintf(stderr, "\n[is_dead_action()] print action:\n");
            // print_action(action);


            safe_place_object(nextRenderedMap, o_pos, ' '); // remove 'x'.


            // fprintf(stderr, "\n[is_dead_action()] after delete the x to be moved:\n\n");
            // print_map(nextRenderedMap); 



            if(is_dead_corner(nextRenderedMap, x_pos)){
                // fprintf(stderr, "\n[is_dead_action()] is_dead_corner.\n\n");
                return true;
            }
            // else if(is_pushPos_unreachable(o_pos, x_pos)){
            //     fprintf(stderr, "\n[is_dead_action()] is_pushPos_unreachable.\n\n");
            //     return true;
            // }  

            return false;         
        }



        void copy_map(char* fromMap, char* toMap){
            for(int i=0; i<fileLen; i++){
                toMap[i] = fromMap[i];
            }
        }


        bool is_dead_corner(char* renderedMap, Pos x_pos){
            // Fast path
            // - a box pushed into corner (2 '#' or 1 '#' + 1 'x') and is not on '.' 

            int dir1, dir2;
            Pos probe_pos, probe_pos1, probe_pos2;
            char mapObj, mapObj_adj1, mapObj_adj2, mapObj_x_pos;
            bool is_dead;

            mapObj_x_pos = get_map_object(renderedMap, x_pos);

            for(int dir=0; dir<=3; dir++){

                // fprintf(stderr, "\n[is_dead_corner()] dir: %d.\n\n", dir);



                dir1 = dir; 
                dir2 = (dir1 + 1)%4;

                move(x_pos, (Dir)dir1, &probe_pos1);
                mapObj_adj1 = get_map_object(renderedMap, probe_pos1);

                move(x_pos, (Dir)dir2, &probe_pos2);
                mapObj_adj2 = get_map_object(renderedMap, probe_pos2);


                if(mapObj_adj1 == '#' && mapObj_adj2 == '#' && mapObj_x_pos != '.'){ 
                    // fprintf(stderr, "\n[is_dead_corner()] type 0\n\n");
                    return true;
                }
                else if(mapObj_adj1 == '#' && mapObj_adj2 == 'x' ||
                        mapObj_adj1 == '#' && mapObj_adj2 == 'X' && mapObj_x_pos == ' '){
                    
                    is_dead = true;

                    move(probe_pos2, (Dir)((dir2 + 1)%4), &probe_pos);
                    mapObj = get_map_object(renderedMap, probe_pos);
                    is_dead = is_dead && (mapObj != ' ' && mapObj != '.');

                    move(probe_pos2, (Dir)((dir2 + 3)%4), &probe_pos);
                    mapObj = get_map_object(renderedMap, probe_pos);
                    is_dead = is_dead && (mapObj != ' ' && mapObj != '.');

                    if(is_dead){
                        // fprintf(stderr, "\n[is_dead_corner()] type 1\n\n");
                        return true;
                    } 

                }                   
                else if(mapObj_adj1 == 'x' && mapObj_adj2 == '#' ||
                        mapObj_adj1 == 'X' && mapObj_adj2 == '#' && mapObj_x_pos == ' '){
                    
                    is_dead = true;

                    move(probe_pos1, (Dir)((dir1 + 1)%4), &probe_pos);
                    mapObj = get_map_object(renderedMap, probe_pos);
                    is_dead = is_dead && (mapObj != ' ' && mapObj != '.');

                    move(probe_pos1, (Dir)((dir1 + 3)%4), &probe_pos);
                    mapObj = get_map_object(renderedMap, probe_pos);
                    is_dead = is_dead && (mapObj != ' ' && mapObj != '.');

                    if(is_dead){
                        // fprintf(stderr, "\n[is_dead_corner()] type 2\n\n");
                        return true;
                    } 
                }
                else if((mapObj_adj1 == 'x' && mapObj_adj2 == 'x') ||
                        (mapObj_adj1 == 'x' && mapObj_adj2 == 'X') ||
                        (mapObj_adj1 == 'X' && mapObj_adj2 == 'x') ||
                        (mapObj_adj1 == 'X' && mapObj_adj2 == 'X' && mapObj_x_pos == ' ')){
                    
                    is_dead = true;

                    move(probe_pos1, (Dir)((dir1 + 1)%4), &probe_pos);
                    mapObj = get_map_object(renderedMap, probe_pos);
                    is_dead = is_dead && (mapObj != ' ' && mapObj != '.');

                    move(probe_pos1, (Dir)((dir1 + 3)%4), &probe_pos);
                    mapObj = get_map_object(renderedMap, probe_pos);
                    is_dead = is_dead && (mapObj != ' ' && mapObj != '.');
                    

                    move(probe_pos2, (Dir)((dir2 + 1)%4), &probe_pos);
                    mapObj = get_map_object(renderedMap, probe_pos);
                    is_dead = is_dead && (mapObj != ' ' && mapObj != '.');

                    move(probe_pos2, (Dir)((dir2 + 3)%4), &probe_pos);
                    mapObj = get_map_object(renderedMap, probe_pos);
                    is_dead = is_dead && (mapObj != ' ' && mapObj != '.');

                    if(is_dead){
                        // fprintf(stderr, "\n[is_dead_corner()] type 3\n\n");
                        return true;
                    }                     
                }              
            }     

            return false;

        }
        

        bool is_pushPos_unreachable(Pos o_pos, Pos x_pos){
            // Slow path
            // 1. find all available action on a box
            // 2. get pos needed to do the action
            // 3. use BFS on `map` to see whether those pos's are reachable.
            // 4. if all of pos are unreachable and 'x' is not on '.', return true.

            char* onexMap = new char[fileLen];
            reset_renderedMap(onexMap);
            safe_place_object(onexMap, x_pos, 'x');
            char mapObj;

            fprintf(stderr, "\n[is_pushPos_unreachable()] safe_place x on onexMap, print_map:\n\n");
            print_map(onexMap);




            fprintf(stderr, "\n[is_pushPos_unreachable()] print debug map:\n\n");
            char* debugMap = new char[fileLen];
            reset_renderedMap(debugMap);
            // copy_map(rawMap, debugMap);
            safe_place_object(debugMap, o_pos, 'o');
            safe_place_object(debugMap, x_pos, 'x');
            print_map(debugMap);

            



            queue<Pos> nextPosQueue;
            queue<Pos> pushPosQueue;
            unordered_map<poskey_t, bool> vstdPosTbl;
            
            Pos pushPos, probe_pos, curPos = o_pos;
            nextPosQueue.push(curPos);
                            
            // find push positions (4 at most).
            for(int dir=0; dir<=3; dir++){
                move(x_pos, (Dir)dir, &probe_pos);
                mapObj = get_map_object(onexMap, probe_pos);
                if(mapObj == ' ' || mapObj == '.') {
                    fprintf(stderr, "\n[is_pushPos_unreachable()] action found, dir: %d\n", dir);
                    pushPosQueue.push(probe_pos);
                }    
            }

            // BFS on onexMap starting from o_pos.
            while(!nextPosQueue.empty()){
                curPos = nextPosQueue.front();
                nextPosQueue.pop();
                vstdPosTbl[pos2key(curPos)] = true;

                for(int dir=0; dir<=3; dir++){
                    add_unvisited_nextPos(onexMap, curPos, (Dir)dir, &vstdPosTbl, &nextPosQueue);
                }
            }
            

            // check whether all push positions cannot be found in vstdPosTbl.
            bool is_all_unreachable = true;
            while(!pushPosQueue.empty()){
                pushPos = pushPosQueue.front();
                pushPosQueue.pop();

                is_all_unreachable = is_all_unreachable && !is_pos_visited(&vstdPosTbl, pushPos);
                if(!is_all_unreachable) break;
            }

            return is_all_unreachable;            
        }



        bool is_pos_visited(unordered_map<poskey_t, bool> *vstdPosTbl, Pos pos){
            return !(vstdPosTbl->find(pos2key(pos)) == vstdPosTbl->end());
        }


        bool add_unvisited_nextPos(char *renderedMap, Pos curPos, Dir dir, 
                    unordered_map<poskey_t, bool> *vstdPosTbl, queue<Pos> *nextPosQueue){
            // check whether the position in `dir` is empty space (' ' or '.').
                // If so and if not already explored, add to 'nextPosQueue'.

            // fprintf(stderr, "\n[add_unvisited_nextPos()] dir: %d\n", (int)dir); 

            Pos nextPos;
            char mapObj;

            move(curPos, dir, &nextPos);
            mapObj = get_map_object(renderedMap, nextPos); 
            
            // fprintf(stderr, "[add_unvisited_nextPos()] mapObj: %c\n", mapObj);

            if(mapObj == '.' || mapObj == ' '){
                if(!is_pos_visited(vstdPosTbl, nextPos)){
                    
                    // fprintf(stderr, "[add_unvisited_nextPos()] pos (%d, %d) not visited.\n", nextPos.row, nextPos.col);

                    nextPosQueue->push(nextPos);
                    return true;
                }else{
                    // fprintf(stderr, "[add_unvisited_nextPos()] pos (%d, %d) already visited.\n", nextPos.row, nextPos.col);
                    return false;                    
                }
            }

            return false;

            // fprintf(stderr, "\n\n");
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

        void render_boxPos(char *renderedMap, bitset<64> boxPos){

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

            // fprintf(stderr, "\n[act()]: before action\n\n");
            // print_map(renderedMap);
            // print_state(cur_state);

            char mapObj;
            Pos o_pos, x_pos;
            
            o_pos.row = action.row;
            o_pos.col = action.col;
            move(o_pos, action.dir, &x_pos);

            // replace 'x' in old position by 'o'.
            mapObj = get_map_object(renderedMap, x_pos); 
            if(mapObj == 'x') set_map_object(renderedMap, x_pos, 'o');
            else if(mapObj == 'X') set_map_object(renderedMap, x_pos, 'O');
            else {fprintf(stderr, "[act()] mapObj != 'x' \n"); exit(-1);}

            // place 'x' on its new position. 
            move(x_pos, action.dir, &x_pos);
            mapObj = get_map_object(renderedMap, x_pos); 
            if(mapObj == ' ') set_map_object(renderedMap, x_pos, 'x');
            else if(mapObj == '.') set_map_object(renderedMap, x_pos, 'X');
            else {fprintf(stderr, "[act()] `x` new position not a space. \n"); exit(-1);}

            // fprintf(stderr, "\n[act()]: after action\n\n");
            // print_map(renderedMap);
             

            map2state(renderedMap, next_state);

            // fprintf(stderr, "\n[act()]: after action\n\n");
            // print_state(*next_state);

        }



        void safe_place_object(char* map, Pos pos, char ch){
            char mapObj;
            mapObj = get_map_object(map, pos);

            if(ch == 'o' || ch == 'O'){
                if(mapObj == ' ') set_map_object(map, pos, 'o');
                else if(mapObj == '.') set_map_object(map, pos, 'O');
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
            int offset = 0;
            int row = 0, col = 0;

            for(int i=0; i<fileLen-1;i++){
                
                ch = renderedMap[i]; 

                if(ch == '\n'){ row++; col = 0; continue; }

                if(ch != '#'){ 

                    if(ch == ' ' || ch == '.'){
                        offset++;
                    }
                    else if(ch == 'x' || ch == 'X'){
                        state->boxPos.set(size_t(offset++), true);
                    }
                    else if(ch == 'o' || ch == 'O'){
                        offset++;
                        state->row = row;
                        state->col = col;
                    }       
                }             

                col++;
            }      
        }


        bool is_reachable(State state, Pos pos1, Pos pos2){
            
            // fprintf(stderr, "\n[is_reachable()]:\n\n"); 

            char* renderedMap = new char[fileLen];
            render_boxPos(renderedMap, state.boxPos);  
            char mapObj;

            queue<Pos> nextPosQueue;
            unordered_map<poskey_t, bool> vstdPosTbl;
            
            Pos probe_pos, curPos = pos1;
            nextPosQueue.push(curPos);
                            
            while(!nextPosQueue.empty()){
                curPos = nextPosQueue.front();
                nextPosQueue.pop();
                vstdPosTbl[pos2key(curPos)] = true;

                for(int dir=0; dir<=3; dir++){
                    add_unvisited_nextPos(renderedMap, curPos, (Dir)dir, &vstdPosTbl, &nextPosQueue);
                }
            }

            if(is_pos_visited(&vstdPosTbl, pos2)){

                // if((pos1.row != pos2.row) && (pos1.col != pos2.col)){
                //     State tmpSt;
                //     tmpSt.boxPos = state.boxPos;

                //     fprintf(stderr, "\n[is_reachable()] is visited. print pos1 state:\n\n");
                //     tmpSt.row = pos1.row;
                //     tmpSt.col = pos1.col;
                //     print_state(tmpSt);


                //     fprintf(stderr, "\n[is_reachable()] is visited. print pos2 state:\n\n");
                //     tmpSt.row = pos2.row;
                //     tmpSt.col = pos2.col;
                //     print_state(tmpSt);                    
                // }

                return true;
            }
            else{
                
                // if((pos1.row != pos2.row) && (pos1.col != pos2.col)){
                //     State tmpSt;
                //     tmpSt.boxPos = state.boxPos;

                //     fprintf(stderr, "\n[is_reachable()] not visited. print pos1 state:\n\n");
                //     tmpSt.row = pos1.row;
                //     tmpSt.col = pos1.col;
                //     print_state(tmpSt);


                //     fprintf(stderr, "\n[is_reachable()] not visited. print pos2 state:\n\n");
                //     tmpSt.row = pos2.row;
                //     tmpSt.col = pos2.col;
                //     print_state(tmpSt);             
                // }



                return false;
            }
        }


        void generate_rvrsState(State curState, Action curAction, State* rvrsState){

            char* renderedMap = new char[fileLen];
            render_boxPos(renderedMap, curState.boxPos); 

            Pos ocurPos, xcurPos, oprevPos, xprevPos;
            
            int dir = (int)curAction.dir;
            int opstDir = (dir + 2)%4; 

            ocurPos.row = curState.row;
            ocurPos.col = curState.col;

            move(ocurPos, (Dir)opstDir, oprevPos);
            move(ocurPos, (Dir)dir, xcurPos);
            xprevPos = ocurPos;
            

            safe_place_object(renderedMap, xcurPos, ' ');
            safe_place_object(renderedMap, xprevPos, 'x');
            safe_place_object(renderedMap, oprevPos, 'o');

            map2state(renderedMap, rvrsState);

        }


        void move_by_action(Action action, Pos* pos){

            Pos curPos;
            curPos.row = action.row;
            curPos.col = action.col;
            move(curPos, action.dir, pos);      

        }



        bool get_steps(State state, Pos fromPos, Pos toPos, queue<Dir>* steps){
    
            char* renderedMap = new char[fileLen];
            render_boxPos(renderedMap, state.boxPos);

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

                    if(add_unvisited_nextPos(renderedMap, curPos, (Dir)dir, &vstdPosTbl, &nextPosQueue);){
                        
                        nextPos = nextPosQueue.back();
                        vstdPosTbl[pos2key(nextPos)] = curPosKey;
                    }
                }
            }


            if(is_pos_visited(&vstdPosTbl, toPos)){
                
                Pos curPos, prevPos;
                Dir step;
                poskey_t prevPoskey, curPosKey, nullPosKey = ((MAP_COORD_RSRV << 8) | MAP_COORD_RSRV);
                

                curPos = toPos;
                curPosKey = pos2key(curPos);
                prevPoskey = vstdPosTbl[curPosKey]

                while(prevPoskey != nullPosKey){
                    prevPos = key2pos(prevPoskey);

                    infer_local_dir(prevPos, curPos, &step);
                    steps.push(step);

                    curPos = prevPos;
                    curPosKey = prevPoskey;
                    prevPoskey = vstdPosTbl[curPosKey]
                }
                
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
            
            if(frow = trow){
                if(fcol - tcol == 1){
                    *dir = LEFT;
                }
                else if(tcol - fcol == 1){
                    *dir = RIGHT;
                }
                else{
                    fprintf(stderr, "\n[infer_local_dir()] Invalid input, not adjacent pos's:\n\n");
                    exit(-1);                    
                }
            }
            else if(fcol == tcol){
                if(frow - trow == 1){
                    *dir = DOWN;
                }
                else if(trow - frow == 1){
                    *dir = UP;
                }
                else{
                    fprintf(stderr, "\n[infer_local_dir()] Invalid input, not adjacent pos's:\n\n");
                    exit(-1);                    
                }
            }
            else{
                fprintf(stderr, "\n[infer_local_dir()] Invalid input, pos's not on same row and col:\n\n");
                exit(-1);
            }
        }



        bool is_pos_visited(unordered_map<poskey_t, poskey_t> *vstdPosTbl, Pos pos){
            return !(vstdPosTbl->find(pos2key(pos)) == vstdPosTbl->end());
        }        


        char *map;
        // char *renderedMap;

        int *rowEnds;
        int *rowBegins;

        State state;        
   
        int fileLen;
        int fileLineNum;
        int spaceNum;
        int colMax;
};







class Solver{
    public:

    Solver(char* filename){
        map = new Map(filename);
        map->print_map(map->map);

        State doneState;
        Action doneAction;
        if(explore(&doneState, &doneAction)){
            recover_steps(doneState, doneAction);
        }
    }


    void recover_steps(State doneState, Action doneAction){

        stepsBufSize = 256;
        stepsBuf = new char[stepsBufSize];
        stepsOfst = 0;

        State prevState, curState, rvrsState;
        Action prevAction, curAction;
        int dir, opstDir;


        curState = doneState;
        curAction = doneAction;
        add_one_step(curAction.dir);
        generate_rvrsState(curState, curAction, &rvrsState); // get prev boxPos and player pos.
        // add_intermediate_steps(rvrsState, curState);

        while(find_vstd_prevStateAction(rvrsState, &prevState, &prevAction)){
            
            add_intermediate_steps(prevState, rvrsState);

            curState = prevState;
            curAction = prevAction;
            add_one_step(curAction.dir);
            generate_rvrsState(curState, curAction, &rvrsState); // get prev boxPos and player pos.
            // add_intermediate_steps(prevState, curState);
        }

    }

    
    void add_intermediate_steps(State prevState, State rvrsState){

    }



    bool find_vstd_prevStateAction(State rvrsState, State* prevState, Action* prevAction){
        
        // - Not in vstdStTbl.
        if(!is_in_vstdStTbl(&vstdStTbl, rvrsState.boxPos)){return false;}


        // - In vstdStTbl.
        Pos rvrsPos, prevPos, probePos;
        Action probeAction;

        // get rvrsPos
        rvrsPos.row = rvrsState.row;
        rvrsPos.col = rvrsState.col;

        // get probePos
        probeAction = vstdStTbl[rvrsState.boxPos];
        map->move_by_action(probeAction, &probePos);

        // See whether rvrsPos and probePos are mutually reachable
        if(map->is_reachable(rvrsState, rvrsPos, probePos)){
            
            prevState->boxPos = rvrsState.boxPos;
            prevState->row = probePos.row;
            prevState->col = probePos.col;

            *prevAction = probeAction;
            return true;
        }
        


        // - Not in vstdClidStTbl.
        if(!is_in_vstdClidStTbl(&vstdClidStTbl, rvrsState.boxPos)){return false;}




        // - In vstdClidStTbl.
        ActionNode *cur_ptr = vstdClidStTbl[rvrsState.boxPos];

        while(cur_ptr){

            probeAction = cur_ptr->action;
            map->move_by_action(probeAction, &probePos);
 
            if(map->is_reachable(rvrsState, rvrsPos, probePos)){

                prevState->boxPos = rvrsState.boxPos;
                prevState->row = probePos.row;
                prevState->col = probePos.col;

                *prevAction = probeAction;
                return true;                
            }
            
            cur_ptr = cur_ptr->next;
        }

        return false;
    }


    void generate_rvrsState(State curState, Action curAction, State* rvrsState){
        map->generate_rvrsState(curState, curAction, &rvrsState);
    }


    void add_one_step(Dir dir){

        if(stepsOfst >= stepsBufSize){
            fprintf(stderr, "\n[add_one_step()] stepsBuf[] overflowed:\n\n");
            exit(-1);
        }

        if(dir == UP) stepsBuf[stepsOfst] = 'W';
        else if(dir == RIGHT) stepsBuf[stepsOfst] = 'D';
        else if(dir == DOWN) stepsBuf[stepsOfst] = 'S';
        else if(dir == LEFT) stepsBuf[stepsOfst] = 'A';
        
        stepsOfst++;
    }
    

    int stepsBufSize;
    char* stepsBuf;
    int stepsOfst;



    bool explore(State* doneState, Action* doneAction){
        
        State state, nextState;
        Action curAction;
        list<Action> action_list;

        nextStateQueue.push(map->get_state());
        

        while (!nextStateQueue.empty()) {
            probe_vstdCount++;
            probe_get_nextStateQueue_max_size(nextStateQueue.size());

            state = nextStateQueue.front();
            nextStateQueue.pop();

            // add_visited_state(&vstdStTbl, &vstdClidStTbl, state);    

            map->get_available_actions(state, &action_list); 


            // fprintf(stderr, "\n[explore()] return from get_available_actions(). print action list:\n\n");
            // print_action_list(action_list);

            while(!action_list.empty()){
                
                curAction = action_list.front();
                action_list.pop_front();

                map->act(state, curAction, &nextState);
                
                
                if(map->is_done(nextState)){

                    *doneState = nextState;
                    *doneAction = curAction;

                    fprintf(stderr, "\n[explore()]: done!\n\n"); 
                    map->print_state(nextState);

                    fprintf(stderr, "\n\n[explore()]: Print stat:");
                    probe_print_stat();

                    return true;
                } 

                if(!is_state_visited(&vstdStTbl, &vstdClidStTbl, nextState)){ 
                    // fprintf(stderr, "\n[explore()] nextState not visited, will be add to queue:\n\n");
                    // map->print_state(nextState);
                    add_visited_state(&vstdStTbl, &vstdClidStTbl, nextState, curAction);    
                    nextStateQueue.push(nextState);
                }    
            }
        }

        return false;
    }




    void add_visited_state(unordered_map<bitset<64>, Action> *vstdStTbl, 
                    unordered_map<bitset<64>, ActionNode*> *vstdClidStTbl, State state, Action action){
        
        // Pos pos;
        // pos.row = state.row;
        // pos.col = state.col;

        

        if(!is_in_vstdStTbl(vstdStTbl, state.boxPos)){ // not inside.
            // (*vstdStTbl)[state.boxPos] = pos;
            (*vstdStTbl)[state.boxPos] = action;
        }else{


            // fprintf(stderr, "\n[add_visited_state()] print new state:\n\n");
            // map->print_state(state);

            // fprintf(stderr, "\n[add_visited_state()] print first visited state:\n\n");

            // Pos vstdPos = (*vstdStTbl)[state.boxPos];
            // State vstdState;
            // vstdState.row = vstdPos.row;
            // vstdState.col = vstdPos.col;
            // vstdState.boxPos = state.boxPos;
            
            // map->print_state(vstdState);



            probe_vstdClidCount++;
            insert_ActionNode(vstdClidStTbl, state, action);
        }
    }


    void insert_ActionNode(unordered_map<bitset<64>, ActionNode*> *vstdClidStTbl, State state, Action action){
        
        // Pos pos;
        // pos.row = state.row;
        // pos.col = state.col;

        // ActionNode *cur = new ActionNode;
        // cur->pos = pos;

        ActionNode *cur = new ActionNode;
        cur->action = action;

        if(!is_in_vstdClidStTbl(vstdClidStTbl, state.boxPos)){
            cur->next = NULL;
        }
        else{
            cur->next = (*vstdClidStTbl)[state.boxPos];
        }

        (*vstdClidStTbl)[state.boxPos] = cur;
    }


    bool is_state_visited(unordered_map<bitset<64>, Action> *vstdStTbl, 
                    unordered_map<bitset<64>, ActionNode*> *vstdClidStTbl, State state){



        // key not found -> haven't been visited.
        if(!is_in_vstdStTbl(vstdStTbl, state.boxPos)){return false;}


        // Pos visitedPos = (*vstdStTbl)[state.boxPos];
        Pos vstdPos, preVstdPos;
        Action preVstdAction = (*vstdStTbl)[state.boxPos];
        preVstdPos.row = preVstdAction.row;
        preVstdPos.col = preVstdAction.col;
        map->move(preVstdPos, preVstdAction.dir, vstdPos);

        Pos curPos{.row{state.row}, .col{state.col}};


        // Have same box positions, and mutually 
            // reachable player positions *with the head* -> deemed visited.
        if(map->is_reachable(state, curPos, vstdPos)){return true;}
        

        // key not found -> haven't been visited.
        if(!is_in_vstdClidStTbl(vstdClidStTbl, state.boxPos)){return false;}


  
        ActionNode *cur_ptr = (*vstdClidStTbl)[state.boxPos];

        while(cur_ptr){

            // vstdPos = cur_ptr->pos;
            Action preVstdAction = cur_ptr->action;
            preVstdPos.row = preVstdAction.row;
            preVstdPos.col = preVstdAction.col;
            map->move(preVstdPos, preVstdAction.dir, vstdPos);

            // Have same box positions, and mutually 
                // reachable player positions *with other collided pos*. 
            if(map->is_reachable(state, curPos, vstdPos)){return true;}
            
            cur_ptr = cur_ptr->next;
        }

        return false;
    }
    

    bool is_in_vstdClidStTbl(unordered_map<bitset<64>, ActionNode*> *vstdClidStTbl, 
                                                                            bitset<64> boxPos){
        return !(vstdClidStTbl->find(boxPos) == vstdClidStTbl->end());
    }


    bool is_in_vstdStTbl(unordered_map<bitset<64>, Action> *vstdStTbl, bitset<64> boxPos){
        return !(vstdStTbl->find(boxPos) == vstdStTbl->end());
    }




    unordered_map<bitset<64>, Action> vstdStTbl;
    unordered_map<bitset<64>, ActionNode*> vstdClidStTbl;
    queue<State> nextStateQueue;
    

    Map* map;
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

    Solver solver(argv[1]);
    

    return 0;
}