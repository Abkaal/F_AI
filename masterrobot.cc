#include <vector>
#include <algorithm>

#include "actions/action.h"
#include "actions/mine.h"
#include "actions/move.h"
#include "actions/create.h"

#include "fields/resourcefield.h"

#include "logic/game.h"

#include "jinkou/masterrobot.h"

using namespace std;

using namespace actions;
using namespace logic;
using namespace resources;
using namespace fields;

using namespace jinkou;

void masterrobot::changeMode(){ //PLACEHOLDER?
    CScrooge::changeMode();
}

void masterrobot::remember(){ // you must take your place in The Circle of Life...
    try{
        appointTaichou(this, dynamic_cast<CBob*>(this));
    }
    catch(exceptions::badformatexception &ex){
        game::getgame().getout()<<ex.getmessage()<<std::endl;
        assert(0);
    }
    std::pair<bool,bool> side;
    auto tmp_map=getSurroundings();
    if(tmp_map[{-1,0}]->getrobots(this).size()>0){
        side=std::pair<bool, bool>(true, false);
    }
    if(tmp_map[{1,0}]->getrobots(this).size()>0){
        side=std::pair<bool, bool>(true, true);
    }
    if(tmp_map[{0,-1}]->getrobots(this).size()>0){
        side=std::pair<bool, bool>(false, false);
    }
    if(tmp_map[{0,1}]->getrobots(this).size()>0){
        side=std::pair<bool, bool>(false, true);
    }
    setSide(side);
    initializeTheFirst(side);
    flags.set(0); // so now he knows he's Taichou;
}

std::pair<int,int> masterrobot::searchSurroundings(char co){
    auto tmp_map=getMap(this->getCoords().first, this->getCoords().second, 1);
    std::pair<int,int> target=this->getCoords();
    if(tmp_map[std::pair<int,int>(target.first,target.second)]==co) return std::pair<int,int>(0,0);
    for(int I=target.first-1; I<=target.first+1; ++I){
        if(tmp_map[std::pair<int,int>(I,target.second)]==co) return std::pair<int,int>(I-target.first,0);
    }
    for(int U=target.second-1; U<=target.second+1; ++U){
        if(tmp_map[std::pair<int,int>(target.first,U)]==co) return std::pair<int,int>(0,U-target.second);
    }
    for(int I=target.first-1; I<=target.first+1; ++I){
        for(int U=target.second-1; U<=target.second+1; ++U){
            if(tmp_map[std::pair<int,int>(I,U)]==co) return std::pair<int,int>(I-target.first,U-target.second);
        }
    }
    return std::pair<int,int>(-2,2); // not found;
}


action *masterrobot::claimTheLand(const vector<action *> &possibleactions){
    TSite *new_world = new TSite;
    auto it=find_if(possibleactions.begin(), possibleactions.end(),
        [](actions::action *a){return dynamic_cast<actions::build*>(a)!=NULL;});
    auto buildAction=dynamic_cast<actions::build*>(*it);
    this->quantumstockpile = new CZaiko(this->getCoords(), new_world);
    toSiteMap(std::pair<int,int>(0,0), new_world);
    buildAction->built=this->quantumstockpile;
    this->current_site=new_world;
    new_world->cpt=this;
    return buildAction;
}

actions::action* masterrobot::lookForResources(const std::vector<actions::action*> &possibleactions, int ss){
    // ss parameter defaults to SITE_SIZE, but here we have wider area to scout;
    std::pair<int,int> tmp=this->getCoords();
    std::pair<int,int> delta=std::pair<int,int>(0,0);
    static int steps=ss-1, tmp_ss=ss;
    static short direction=this->getDir();
    static bool start=true;
    // at the beginning I stand on Zaiko;
    if(tmp_ss!=ss){ // if it's not the first loop:
        tmp_ss=ss;
        steps=2; // next loop is wider;
        direction=this->getDir();
    }
    if(steps==0){
        direction+=Signumoid(flags.test(8));
        if(start){
            steps=tmp_ss-1; start=false;
        }
        else steps=2*(tmp_ss-1);
    }
    assert(steps); // ensure steps is not equal to zero (precisely, in this case it's greater);
    //turns--;
    std::pair<int,int> found=std::pair<int,int>(0,0);
    if(flags.test(3)){
        found=searchSurroundings('m');
        if(found!=std::pair<int,int>(-2,2)){
            flags.reset(3);
            this->plan.m.push_back(tmp+found); // OK?
        }
    }
    if(flags.test(4)){
        found=searchSurroundings('d');
        if(found!=std::pair<int,int>(-2,2)){
            flags.reset(4);
            this->plan.d.push_back(tmp+found); // OK?
        }
    }
    if(((vertical(direction))&&(tmp.second==0))||((horizontal(direction))&&(tmp.first==0))){
        // we have to respect the GA, because it's an agreement between two getlemen, not between a gentleman and a klaus xD
        flags.flip(8);
        direction+=Signumoid(flags.test(8));
        steps=2;
    }
    delta=dirToPair(direction);
    //x+=delta.first; y+=delta.second;
    --steps;
    return findMoveAction(possibleactions, delta);
}

actions::action* masterrobot::goRound(const std::vector<actions::action*> &possibleactions){//działa na SITE_SIZE, ale tylko dla 3 xD
    static int s=2, turns=0;
    static short direction=this->getDir();
    static bool start=true; // false as of we start scouting as such;
    std::pair<int,int> delta=std::pair<int,int>(0,0);
    ++turns;
    if(turns==16) flags.set(1); // tu skończyliśmy, czy jeszcze musimy jeden MOVE wykonać?
    if(s==0){
        direction++;
        if(start){
            s=SITE_SIZE-1; start=false;
        }
        else s=2*(SITE_SIZE-1);
    }
    assert(s);
    
    delta=dirToPair(direction);
    --s;
    return findMoveAction(possibleactions, delta);

    // docelowo zrobić tę funkcję tak, żeby potrafiła sobie z większym site poradzić;
}

void masterrobot::manageTeam(){
    // we're in phase 2, so we're sure we are threesome;
    bool out=true;
    TOrders* o;
    readReports();
    if(this->brigade->McDuck->isBusy() || this->brigade->Glomgold->isBusy()) out=false; // OK, czy przepisać na gold_mined???
    if(out) {
        if(!scouts && zaiko_ready) changeSite();
        else{
            if(this->brigade->builder && this->brigade->Glomgold &&
               !this->brigade->McDuck->isBusy() && !this->brigade->builder->isBusy() &&
               !this->brigade->Glomgold->isBusy()){
                std::vector<CMyRobot*> scouting;
                scouting.push_back(this->brigade->Glomgold);
                scouting.push_back(this->brigade->McDuck);
                scouting.push_back(this->brigade->builder);
                sendScouting(scouting);
            }
        }
    }
    else{ // we don't change site:
        // no need to build orders - everybody knows what is to be done xD
        logic::resourceset brigade_cost;
        brigade_cost+=getCost(ZAIKO);
        for(int i=0;i<3;i++){
            brigade_cost+=getCost(BRIGADE_TEMPLATE.at(i));
        }
        if(this->current_site->stockpile->getloadedresources()>brigade_cost && !flags.test(6)){
            // now we are able to create a new brigade and move on to phase 3:
            o = new TOrders;
            o->o="create";
            o->make_it = new std::vector<ERobot>(BRIGADE_TEMPLATE);
            this->brigade->builder->takeOrders(o, this);
            flags.set(6);
        }
    }
}

action *masterrobot::earlyGame(const vector<action *> &possibleactions){
    if(starting_offset>0){
        --starting_offset;
        return findMoveAction(possibleactions, dirToPair(this->dir));
    }
    /*
    BATTLE PLAN:
    szukamy złota => rypiemy olej (2 kursy), 2 kursy złota, 2 daimonda, 2 metalu
    Bob robi 2 kursy z metalem && master 2 oleju
    robimy Flintheart'a faza I ~> faza II
    */
    if(!this->quantumstockpile){
        std::pair<int,int> o=std::pair<int,int>(-2,-2);
        if(this->getloadedresources().get<oil>()<50) o=searchSurroundings('o');
        std::pair<int,int> gold=searchSurroundings('g');
        if(o.first>-2){
            if(o.first) return findMoveAction(possibleactions, std::pair<int,int>(o.first, 0));
            if(o.second)return findMoveAction(possibleactions, std::pair<int,int>(0, o.second));
            return findMineAction(possibleactions);
        }
        if(gold.first>-2){
            if(gold.first) return findMoveAction(possibleactions, std::pair<int,int>(gold.first, 0));
            if(gold.second)return findMoveAction(possibleactions, std::pair<int,int>(0, gold.second));
            return claimTheLand(possibleactions);
        }
    }
    if(this->getloadedresources().get<oil>()<50 && !flags.test(2)){
        std::pair<int,int> o=searchSurroundings('o');
        if(o.first>-2){
            if(o.first) return findMoveAction(possibleactions, std::pair<int,int>(o.first, 0));
            if(o.second)return findMoveAction(possibleactions, std::pair<int,int>(0, o.second));
            return findMineAction(possibleactions);
        }
    }
    if(!flags.test(1)) return goRound(possibleactions); // gdy wywołuję to pierwszy raz, stoję na Zaiko, jo?
    if(!flags.test(2)){
        this->plan=this->planSomething();
        flags.set(2);
        --this->dir; // Shougun once changes his dir;
    }
    delete go_to; go_to=NULL; // this doesn't depend on Bob;
    std::pair<int,int> delta;
    static int ss=SITE_SIZE+2; // this will be used, if we didn't find sth on the starting Site;
    static int t=8*(ss-1)+2;
    if(!flags.test(5)){ // so we didn't have Bob:
        if(+this->getloadedresources()>=300) return drop(possibleactions);
        if(this->quantumstockpile->getloadedresources().get<oil>()<600){ // he mines oil:
            this->go_to = new std::pair<int,int>(this->plan.o.front());
            delta=this->whereAreWeGoingFromHere(*this->go_to);
            if(delta!=std::pair<int, int>(0, 0)) return findMoveAction(possibleactions, delta);
            return findMineAction(possibleactions);
        }
        if(this->quantumstockpile->getloadedresources().get<gold>()<600){ // he mines gold:
            this->go_to = new std::pair<int,int>(this->plan.g.front());
            delta=this->whereAreWeGoingFromHere(*this->go_to);
            if(delta!=std::pair<int, int>(0, 0)) return findMoveAction(possibleactions, delta);
            return findMineAction(possibleactions);
        }
        if(!plan.d.empty()){
            if(this->quantumstockpile->getloadedresources().get<diamond>()<600){ // he mines diamond:
                this->go_to = new std::pair<int,int>(this->plan.d.front());
                delta=this->whereAreWeGoingFromHere(*this->go_to);
                if(delta!=std::pair<int, int>(0, 0)) return findMoveAction(possibleactions, delta);
                return findMineAction(possibleactions);
            }
        }
        else flags.set(4);
        if(plan.m.size()>1){ // we must be sure we can afford building BOTH Bob and Flintheart;
            if(this->quantumstockpile->getloadedresources().get<metal>()<600){ // he mines metal:
                this->go_to = new std::pair<int,int>(this->plan.m.front());
                delta=this->whereAreWeGoingFromHere(*this->go_to);
                if(delta!=std::pair<int, int>(0, 0)) return findMoveAction(possibleactions, delta);
                return findMineAction(possibleactions);
            }
        }
        else flags.set(3);
        if(flags.test(3)||flags.test(4)){
            if(t==0){
                ss+=2;
                t=8*(ss-1)+2;
            }
            else t--;
            return lookForResources(possibleactions,ss);
            ss+=2;
        }
        delta=whereAreWeGoingFromHere(this->quantumstockpile->getCoords());
        if(delta!=std::pair<int,int>(0,0)) return findMoveAction(possibleactions, delta);
        auto it=find_if(possibleactions.begin(), possibleactions.end(),
            [](actions::action *a){return dynamic_cast<actions::create*>(a)!=NULL;});
        auto createAction=dynamic_cast<actions::create*>(*it);
        robots::robot* r;
        r = new CBob(this->getCoords());
        createAction->created=r;
        createAction->oil_to_tank=50;
        assert(createAction->verifyparameters());
        flags.set(5);
        TOrders *o = new TOrders;
        o->o="mine";
        o->stockpile=quantumstockpile;
        o->more_coords=new std::vector<std::pair<int,int>>(this->plan.m);
        dynamic_cast<CBob*>(r)->takeOrders(o, this);
        try{
            if(!this->getBrigade(this)) {
                TBrigade* brig=new TBrigade;
                this->setBrigade(brig,this);
            }
            this->assignToBrigade(this->getBrigade(this),this,SCROOGE);
            this->assignToBrigade(this->getBrigade(this),r,BOB);
        }
        catch(exceptions::badformatexception &ex){
            game::getgame().getout()<<ex.getmessage()<<std::endl;
            assert(0); // bo jak coś pieprznie w try, niech WSZYSTKO pieprznie;
        }
        return createAction;
    }
    else{ // we have Bob, so we'll create a Flintheart:
        if(this->quantumstockpile->getloadedresources().get<oil>()<800 || this->quantumstockpile->getloadedresources().get<metal>()<600){
            this->go_to = new std::pair<int,int>(this->plan.o.front());
            delta=this->whereAreWeGoingFromHere(*this->go_to);
            if(delta!=std::pair<int, int>(0, 0)) return findMoveAction(possibleactions, delta);
            return findMineAction(possibleactions);
        }
        delta=whereAreWeGoingFromHere(this->quantumstockpile->getCoords());
        if(delta!=std::pair<int,int>(0,0)) return findMoveAction(possibleactions, delta);
        auto it=find_if(possibleactions.begin(), possibleactions.end(),
            [](actions::action *a){return dynamic_cast<actions::create*>(a)!=NULL;});
        auto createAction=dynamic_cast<actions::create*>(*it);
        robots::robot* r;
        r = new CFlintheart(this->getCoords(),this->quantumstockpile);
        createAction->created=r;
        createAction->oil_to_tank=50;
        assert(createAction->verifyparameters());
        //this->phase=2; // commented out as this is done by Shougun;
        TOrders *o = new TOrders;
        o->o="mine";
        o->stockpile=quantumstockpile;
        o->more_coords=new std::vector<std::pair<int,int>>(this->plan.d);
        dynamic_cast<CFlintheart*>(r)->takeOrders(o, this);
        try{
            this->assignToBrigade(this->getBrigade(this),r,FLINTHEART);
        }
        catch(exceptions::badformatexception &ex){
            game::getgame().getout()<<ex.getmessage()<<std::endl;
            assert(0);
        }
        return createAction; // celowo za asercją, j/w.
    }
}

action* masterrobot::earlyMidGame(const vector<action *> &possibleactions){
    this->manageTeam();
    this->changeMode();
    return CScrooge::defaultBehaviour(possibleactions);
}

action* masterrobot::midGame(const vector<action *> &possibleactions){
    return CScrooge::chooseAction(possibleactions); // just Scrooge;
}

action *masterrobot::chooseAction(const vector<action *> &possibleactions) {
    if(!knowHeIsTaichou) remember(); // the very beginning of the game;
    CShougun::shougunTasks();
    toMap(getSurroundings(), getCoords());
    //CShougun::readReports();
    //if(this->phase==1) return earlyGame(possibleactions);
    //else return midGame(possibleactions); // change it using master's manageTeam();
    switch(this->phase){
        case 1: return earlyGame(possibleactions); break;
        case 2: return earlyMidGame(possibleactions); break;
        default: return midGame(possibleactions); break;
    }
}

void jinkou::init_masterrobot() {
    addmasterrobot("jinkou", new masterrobot);
}