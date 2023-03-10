#include <stdio.h>
#include <stdlib.h>
// #include <string.h>

#include <iostream>
#include <fstream>
#include <string>
#include <bitset>

#include <list>
#include <queue>
#include <unordered_map>

// using namespace std;



struct State{
    bitset<64> boxPos;
    char row, col;
};

struct Pos{
    char row, col;
};

enum Dir{
    up,
    right,
    down,
    left
};

struct Action{
    char row, col;
    Dir dir;
};


class Map{
    public:

        Map(char* filename){
            
            read_map(filename);
        }

        void print_map(){
            
            fprintf(stderr, "\n[print_map()]:\n\n");

            for (int i=0; i<fileLen; i++){
                fprintf(stderr, "%c", map[i]);
            }
        }    


        void print_map_state(){
            
            fprintf(stderr, "\n[print_map_state()]:\n\n");

            char ch;
            int offset = 0;
            int row = 0, col = 0;

            for (int i=0; i<fileLen-1; i++){
                ch = map[i];
                
                if(ch == '\n'){
                    row++; col = 0;
                    // continue;
                }
                else{

                
                    if(ch == ' '){
                        if(state.boxPos[offset++] == 1){
                            ch = 'x';
                        }
                        else if(row == state.row && col == state.col){
                            ch = 'o';
                        }
                    }
                    else if(ch == '.'){
                        if(state.boxPos[offset++] == 1){
                            ch = 'X';
                        } 
                        else if(row == state.row && col == state.col){
                            ch = 'O';
                        }                    
                    }

                    col++;
                
                }

                fprintf(stderr, "%c", ch);

            }




            // fprintf(stderr, "\n\n[test get_map_object()]:\n\n");

            // Pos pos;
            // char obj;

            // for(int i =0; i<9; i++){
            //     for(int j =0; j<10; j++){
            //         pos.row = i; pos.col = j;
            //         fprintf(stderr, "%c", get_map_object(pos));
            //     }
            // }
        }    


        void _get_file_info(char* filename){

            FILE* ptr;
            char ch;
        
            ptr = fopen(filename, "r");
            if (NULL == ptr) {
                fprintf(stderr, "file can't be opened \n");
                exit(-1);
            }            

            fileLineNum = 0;
            fileLen = 0;           
            spaceNum = 0; 
            ch = fgetc(ptr);
            while (ch != EOF){  
                fileLen++;
                if(ch == '\n') fileLineNum++;
                else if(ch != '\n' && ch != EOF && ch != '#') spaceNum++;
                fprintf(stderr, "%c", ch);
                ch = fgetc(ptr);
            }

            fprintf(stderr, "\n");
            fprintf(stderr, "fileLen: %d\n", fileLen);
            fprintf(stderr, "fileLineNum: %d\n", fileLineNum);
            fprintf(stderr, "spaceNum: %d\n", spaceNum);

            fclose(ptr);

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
                else if(ch == 'X' || ch == '.'){
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

        }



        void get_available_actions(State state, list<Action>* action_list){
            // Do BFS to find what `action`'s are available given `state`.

            char* renderedMap = new char[fileLen];
            render_boxPos(renderedMap, state.boxPos);

            queue<Pos> nextPosQueue;
            unordered_map<Pos, bool> vistedPos;

            Pos curPos{.row{state.row}, .col{state.col}};
            Pos nextPos;
            Action action;
            nextPosQueue.push(curPos);

            while(!nextPosQueue.empty()){
                curPos = nextPosQueue.front();
                nextPosQueue.pop();
                vistedPos[curPos] = true;

                add_local_available_action(renderedMap, curPos, &action_list);
                
                for(int dir=0; dir<=3; dir++){
                    check_move(renderedMap, curPos, (Dir)dir, &vistedPos, &nextPosQueue);
                }
            }
        }


        bool willbe_dead_state(char* renderedMap, Action action){

            Pos curPos{.row{action.row}, .col{action.col}};
            Pos o_pos, x_pos, probe_pos;

            char mapObj;
            
            move(curPos, action.dir, &o_pos);
            move(o_pos, action.dir, &x_pos);

            if(){
                // Fast path
                // - a box pushed into corner (2 '#' or 1 '#' + 1 'x') and is not on '.'  
                int dir1, dir2;
                Pos probe_pos1, probe_pos2;
                char mapObj_adj1, mapObj_adj2;
                bool is_dead = true;

                // mapObj_adj = get_map_object(renderedMap, x_pos);


                for(int dir=0; dir<=3; dir++){
                    
                    dir1 = dir; 
                    dir2 = (dir1 + 1)%4;

                    move(x_pos, dir1, &probe_pos1);
                    mapObj_adj1 = get_map_object(renderedMap, probe_pos1);

                    move(x_pos, dir2, &probe_pos2);
                    mapObj_adj1 = get_map_object(renderedMap, probe_pos2);

                    if(mapObj_adj1 == '#' && mapObj_adj2 == '#' && mapObj_adj != '.'){
                        return true;
                    }
                    else if(mapObj_adj1 == '#' && mapObj_adj2 == 'x'){

                        move(probe_pos2, (Dir)((dir2 + 1)%4), &probe_pos);
                        mapObj_adj = get_map_object(renderedMap, probe_pos);
                        is_dead = is_dead && (mapObj_adj != ' ' && mapObj_adj != '.');

                        move(probe_pos2, (Dir)((dir2 + 3)%4), &probe_pos);
                        mapObj_adj = get_map_object(renderedMap, probe_pos);
                        is_dead = is_dead && (mapObj_adj != ' ' && mapObj_adj != '.');

                        return is_dead;
   
                    }
                    else if(mapObj_adj1 == '#' && mapObj_adj2 == 'X'){

   
                    }                    
                    else if(mapObj_adj1 == 'x' && mapObj_adj2 == '#'){

                        move(probe_pos1, (Dir)((dir2 + 1)%4), &probe_pos);
                        mapObj_adj = get_map_object(renderedMap, probe_pos);
                        if(mapObj_adj != ' ' && mapObj_adj != '.') is_dead = is_dead && true;

                        move(probe_pos1, (Dir)((dir2 + 3)%4), &probe_pos);
                        mapObj_adj = get_map_object(renderedMap, probe_pos);
                        if(mapObj_adj != ' ' && mapObj_adj != '.') is_dead = is_dead && true;

                        return is_dead;
    
                    }
                    else if(mapObj_adj1 == 'x' && mapObj_adj2 == 'x'){

                        move(probe_pos2, (Dir)((dir2 + 1)%4), &probe_pos);
                        mapObj_adj = get_map_object(renderedMap, probe_pos);
                        if(mapObj_adj != ' ' && mapObj_adj != '.') is_dead = is_dead && true;

                        move(probe_pos2, (Dir)((dir2 + 3)%4), &probe_pos);
                        mapObj_adj = get_map_object(renderedMap, probe_pos);
                        if(mapObj_adj != ' ' && mapObj_adj != '.') is_dead = is_dead && true;
   
                        move(probe_pos1, (Dir)((dir2 + 1)%4), &probe_pos);
                        mapObj_adj = get_map_object(renderedMap, probe_pos);
                        if(mapObj_adj != ' ' && mapObj_adj != '.') is_dead = is_dead && true;

                        move(probe_pos1, (Dir)((dir2 + 3)%4), &probe_pos);
                        mapObj_adj = get_map_object(renderedMap, probe_pos);
                        if(mapObj_adj != ' ' && mapObj_adj != '.') is_dead = is_dead && true;

                        return is_dead;
                    }                    
                }
            }
            else if(){
                // Fast path
                // - 4 box form square, not all of them are on '.'

            }
            else{
                // Slow path
                // 1. find all available action on a box
                // 2. get pos needed to do the action
                // 3. use BFS on `map` to see whether those pos's are reachable.
                // 4. if all of pos are unreachable and 'x' is not on '.', return true.

                char* rawMap = new char[fileLen];
                reset_renderedMap(rawMap);

                queue<Pos> nextPosQueue;
                queue<Pos> pushPosQueue;
                unordered_map<Pos, bool> vistedPos;
                
                curPos = o_pos;
                nextPosQueue.push(curPos);
                                
                for(int dir=0; dir<=3; dir++){
                    move(x_pos, (Dir)dir, &probe_pos);
                    mapObj = get_map_object(rawMap, probe_pos);
                    if(mapObj == ' ' || mapObj == '.') pushPosQueue.push(probe_pos);
                }


                while(!nextPosQueue.empty()){
                    curPos = nextPosQueue.front();
                    nextPosQueue.pop();
                    vistedPos[curPos] = true;

                    for(int dir=0; dir<=3; dir++){
                        check_move(rawMap, curPos, (Dir)dir, &vistedPos, &nextPosQueue);
                    }
                }
                
                bool is_all_unreachable = true;
                while(!pushPosQueue.empty()){
                    pushPos = pushPosQueue.front();
                    pushPosQueue.pop();

                    is_all_unreachable = is_all_unreachable && !is_pos_visited(&vistedPos, pushPos);
                    if(!is_all_unreachable) break;
                }

                return is_all_unreachable;
            }            
        }


        void add_local_available_action(char* renderedMap, Pos curPos, list<Action>* action_list){
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
                        if(!willbe_dead_state(renderedMap, action)) action_list->push_back(action);
                    }
                }
            }
        }


        bool is_pos_visited(unordered_map<Pos, bool> *vistedPos, Pos pos){
            return vistedPos.find(pos) == vistedPos.end();
        }


        bool check_move(char *renderedMap, Pos curPos, Dir dir, 
                    unordered_map<Pos, bool> *vistedPos, queue<Pos> *nextPosQueue){
            // check whether the position in `dir` is empty space (' ' or '.').
                // If so and if not already explored, add to 'nextPosQueue'.

            Pos nextPos;
            char mapObj;

            move(curPos, dir, &nextPos);

            mapObj = get_map_object(renderedMap, nextPos); 
            if(mapObj == '.' || mapObj == ' '){
                if(!is_pos_visited(vistedPos, nextPos)){
                    nextPosQueue->push(nextPos);
                }
            }
        }

        void move(Pos curPos, Dir dir, Pos *nextPos){

            nextPos->row = curPos.row;
            nextPos->col = curPos.col;

            if(dir == up) nextPos->row -= 1;
            else if(dir == right) nextPos->col += 1;
            else if(dir == down) nextPos->row += 1;
            else if(dir == left) nextPos->col -= 1;
        }

        void act(State* state, Action action){

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

        char *map;
        // char *renderedMap;

        int *rowEnds;
        int *rowBegins;

        State state;        
   
        int fileLen;
        int fileLineNum;
        int spaceNum;
};


class Solver{
    public:

    Solver(char* filename){
        map = new Map(filename);
        map->print_map();
        map->print_map_state(); 

        nextStateQueue.push(map.get_state());
    }

    void exploreOneStep(){
        State state = nextStateQueue.front();
        nextStateQueue.pop();

        vistedBoxPos[state.boxPos] = true;

        list<Action> action_list;
        get_available_actions(state, &action_list);

        State nextState;
        while(!action_list.empty()){
            nextState = state;
            map->act(&nextState, action_list.front());
            
            if(map->is_done(nextState)){
                done = true;
                break;
            } 

            if(!is_boxPos_visited(nextState.boxPos)){
                nextStateQueue.push(nextState);
            }
        }
    }


    void explore(){
        done = false;
        while (!done) {
            exploreOneStep();
        }
    }


    bool is_boxPos_visited(bitset<8> boxPos){
        return vistedBoxPos.find(boxPos) == vistedBoxPos.end();
    }


    Map* map;
    bool done;
    queue<State> nextStateQueue;
    unordered_map<bitset<8>, bool> vistedBoxPos;


};





int main(int argc, char** argv) {
    
    fprintf(stderr, "argv[1]: %s\n", argv[1]);

    Solver solver(argv[1]);

    return 0;
}