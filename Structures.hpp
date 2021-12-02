#ifndef STRUCTURES_HPP
#define STRUCTURES_HPP

#include<unordered_map>
#include<algorithm>
#include<cassert>

#include "actions/mine.h"
#include "actions/move.h"
#include "actions/transfer.h"

namespace jinkou{
    
    //class CMyRobot;
    class CMyMiner;
    class CScrooge;
    class CChuck;
    class CFlintheart;
    class CBob;
    class CNotBob;
    class CRockerduck;
    class CGajeel;
    class CRaba;
    class CTaichou;
    class CShougun;
    class CZaiko;
    
    const int ROBOT_TYPES=8;
    const int TURNS=1000;
    /* ATTENTION:
    only odd values of SITE_SIZE make sense;
    but even ones make scouting more effective :P
    the minimal sensible value is 3;
    */
    const int SITE_SIZE=3;
    const short SCOUTS=3;
    const int ENDGAME=4;
    
    inline std::pair<int,int> operator+(std::pair<int,int> lhs, const std::pair<int,int>& rhs){
        return std::pair<int,int>(lhs.first+rhs.first, lhs.second+rhs.second);
    }
    
    inline std::pair<int,int> operator*(std::pair<int,int> lhs, const std::pair<int,int>& rhs){
        return std::pair<int,int>(lhs.first*rhs.first, lhs.second*rhs.second);
    }
    
    inline std::vector<std::pair<int,int>> sort_by_distance(std::vector<std::pair<int,int>>& the_vector, std::vector<int>& dist){
        std::vector<std::pair<int,int>> vout;
        int min=dist.front();
        unsigned int i;
        while((!the_vector.empty())&&(!dist.empty())){
            for(i=1;i<dist.size();i++){
                if(dist.at(i)<min) min=dist.at(i);
            }
            vout.push_back(the_vector.at(i));
            the_vector.erase(the_vector.begin()+i);
            dist.erase(dist.begin()+i);
        }
        return vout;
    }
    
    inline int distance(std::pair<int,int> a, std::pair<int,int> b){
        return abs(a.first-b.first)+abs(a.second-b.second);
    }
    
    inline std::vector<int> distance_to_point(std::vector<std::pair<int,int>> the_vector, std::pair<int,int> the_point){
        std::vector<int> output;
        for(auto point:the_vector){
            output.push_back(distance(point, the_point));
        }
        return output;
    }
    
    // bool pathfinding_* - functions used in sorting vectors for Scrooge (and maybe Flintheart):
    inline bool pathfinding_asc_first(std::pair<int,int> a, std::pair<int,int> b){
        return a.first<b.first;
    }
    
    inline bool pathfinding_desc_first(std::pair<int,int> a, std::pair<int,int> b){
        return a.first>b.first;
    }
    
    inline bool pathfinding_asc_second(std::pair<int,int> a, std::pair<int,int> b){
        return a.second<b.second;
    }
    
    inline bool pathfinding_desc_second(std::pair<int,int> a, std::pair<int,int> b){
        return a.second>b.second;
    }
    
    inline int Signumoid(bool in){
        if(in) return 1;
        return -1;
    }
    
    inline bool vertical(short& dir){
        while(dir<0) dir+=4; // foolproofness;
        return(dir%2); // 1 and 3 are vertical;
    }
    
    inline bool horizontal(short& dir){
        return !vertical(dir); // 0 and 2 are horizontal;
    }
    
    inline void Invert(logic::resourceset &rs){
        rs.set<resources::oil>(rs.get<resources::oil>()*-1);
        rs.set<resources::metal>(rs.get<resources::metal>()*-1);
        rs.set<resources::diamond>(rs.get<resources::diamond>()*-1);
        rs.set<resources::gold>(rs.get<resources::gold>()*-1);
    }
    
    inline std::pair<int,int> dirToPair(short& dir){
        std::pair<int,int> delta;
        while(dir<0) dir+=4; // foolproofness;
        switch(dir%4){
            case 0: delta.first=0; delta.second=-1; break;
            case 1: delta.first=0; delta.second=1; break;
            case 2: delta.first=1; delta.second=0; break;
            case 3: delta.first=0; delta.second=-1; break;
            default: delta.first=0; delta.second=0; break;
        }
        return delta;
    }
    
    inline actions::action* verifyAction(actions::action* a){
        assert(a->verifyparameters());
        return a;
    }
    
    inline actions::action* findMineAction(const std::vector<actions::action*> &possibleactions){
        auto it=find_if(possibleactions.begin(), possibleactions.end(),
        [](actions::action *a){return dynamic_cast<actions::mine*>(a)!=NULL;});
        auto mineAction=dynamic_cast<actions::mine*>(*it);
        return verifyAction(mineAction);
    }
    
    inline actions::action* findMoveAction(const std::vector<actions::action *> &possibleactions, std::pair<int,int> p, bool verify=true){
        auto it=find_if(possibleactions.begin(), possibleactions.end(),
            [](actions::action *a){return dynamic_cast<actions::move*>(a)!=NULL;});
        auto moveAction=dynamic_cast<actions::move*>(*it);
        moveAction->delta=p;
        if(verify) return verifyAction(moveAction);
        else return moveAction;
    }
    
    inline actions::action* findTransferAction(const std::vector<actions::action*> &possibleactions, robots::robot* s,robots::robot* t, logic::resourceset rs){
        auto it=find_if(possibleactions.begin(), possibleactions.end(),
        [](actions::action *a){return dynamic_cast<actions::transfer*>(a)!=NULL;});
        auto transferAction=dynamic_cast<actions::transfer*>(*it);
        transferAction->source=s;
        transferAction->target=t;
        transferAction->totransfer=rs;
        return verifyAction(transferAction);
    }
    
    enum ERobot{
        SCROOGE, CHUCK, FLINTHEART, BOB, NOTBOB, ROCKERDUCK, GAJEEL, RABA, ZAIKO //it SHOULDN'T be, but is
    };
    
    const std::vector<ERobot> BRIGADE_TEMPLATE({SCROOGE,BOB,FLINTHEART,ROCKERDUCK,RABA,RABA,RABA,RABA,RABA,RABA,RABA,RABA});
    
    enum ECodes{
        MINE, MOVE, BUILD, CREATE, AUTO, FETCH, SCOUT, SUPERSCOUT, N
    };
    
    const std::unordered_map<std::string, ECodes> order_codes={
        {"mine", MINE}, {"move", MOVE}, {"build", BUILD}, {"create", CREATE}, {"auto", AUTO},
        {"fetch", FETCH}, {"scout", SCOUT}, {"superscout", SUPERSCOUT}, {"n", N}
    };
    
    inline logic::resourceset getCost(ERobot robot, int ott=50){
        logic::resourceset rs;
        switch(robot){
            case SCROOGE:
                rs.set<resources::oil>(225);
                rs.set<resources::metal>(400);
                rs.set<resources::gold>(200);
                break;
            case BOB:
                rs.set<resources::oil>(225);
                rs.set<resources::metal>(600);
                rs.set<resources::diamond>(200);
                rs.set<resources::gold>(200);
                break;
            case FLINTHEART:
                rs.set<resources::oil>(225);
                rs.set<resources::metal>(600);
                rs.set<resources::diamond>(200);
                rs.set<resources::gold>(200);
                break;
            case GAJEEL:
                rs.set<resources::oil>(225);
                rs.set<resources::metal>(600);
                rs.set<resources::gold>(200);
                break;
            case CHUCK:
                rs.set<resources::oil>(225);
                rs.set<resources::metal>(400);
                rs.set<resources::diamond>(200);
                rs.set<resources::gold>(200);
                break;
            case ROCKERDUCK:
                rs.set<resources::oil>(425);
                rs.set<resources::metal>(400);
                rs.set<resources::gold>(200);
                break;
            case RABA:
                rs.set<resources::oil>(225);
                rs.set<resources::metal>(200);
                break;
            case ZAIKO:
                rs.set<resources::oil>(-ott);
                rs.set<resources::metal>(100);
                rs.set<resources::gold>(75);
                break;
            case NOTBOB: assert(0); // FUN! XD
        }
        rs.set<resources::oil>(rs.get<resources::oil>()+ott);
        return rs;
    }
    
    inline logic::resourceset getCost(const std::vector<ERobot> in_r, int ott=50){
        logic::resourceset rs;
        for(auto& x:in_r) rs+=getCost(x,ott);
        return rs;
    }
    
    struct TBrigade{
        ERobot freshman=NOTBOB; //freshman
        CScrooge* McDuck;
        std::vector<CChuck*> Norris;
        CBob* builder;
        CFlintheart* Glomgold;
        std::vector<CRockerduck*> Rockerfeller;
        std::vector<CGajeel*> RedFox;
        std::vector<CRaba*> herd;
        bool complete=false;
        bool ready=false;
    };
    
    struct TSite{
        CTaichou* cpt=NULL;
        CZaiko *stockpile=NULL;
    };
    
    struct TGrandReport{
        logic::resourceset needs;
        char need_robots[ROBOT_TYPES];
        bool moving=false;
        bool important=false;
    };
    
    struct TGrandOrders{
        ERobot make_it; // who to create;
        CTaichou* cpt=NULL; // the cpt for created robot;
        logic::resourceset needed=logic::resourceset(0);
        CZaiko* w=NULL;
        bool nohelpforyou=false;
    };
    
    struct TReport{
        ECodes type=N;
    };
    
    struct TMineReport{
        char what_mined='e';
        bool mined=false;
    };
    
    struct TScoutReport: TReport{
        bool finished=false;
    };
    
    struct TCreateReport: TReport{
        ERobot what_made=NOTBOB;
        bool failed=false;
        bool swapped=false;
        bool finished=false;
    };
    
    struct TBuildReport: TReport{
        bool done=false;
    };
    
    struct TOrders{
        std::string o="n"; // there's no better idea;
        std::pair<int, int> *coords=NULL;
        std::pair<int, int> *coords2=NULL; // used usually in from-to orders;
        std::vector<std::pair<int, int>> *more_coords=NULL; // to allow Taichou to plan the work for a miner in advance;
        logic::resourceset *shopping_list=NULL;
        std::vector<ERobot> *make_it=NULL;
        CMyMiner *master=NULL;
        CZaiko *stockpile=NULL;
        bool change_site=false;
    };
    
    struct TPlan{
        std::vector<std::pair<int,int>> o;
        std::vector<std::pair<int,int>> m;
        std::vector<std::pair<int,int>> d;
        std::vector<std::pair<int,int>> g;
    };
    
    struct TScoutingData{
        int steps=2;
        short direction=1; // we DO NOT determine it for each scout individually;
        int turns=18; // for the first loop;
        int tmp_ss=3;
        bool got_to_zaiko=false; // they have to got to the new Zaiko before they start thinking about scouting;
        bool started=false;
    };
    
}

#endif