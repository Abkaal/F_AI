#include <vector>
#include <algorithm>

#include "actions/move.h"

#include "fields/resourcefield.h"

#include "logic/resourceset.h"

#include "jinkou/Shikikan.hpp"
#include "jinkou/CZaiko.hpp"
#include "jinkou/CBob.hpp"
#include "jinkou/CFlintheart.hpp"
#include "jinkou/CScrooge.hpp"
#include "jinkou/CRaba.hpp"
#include "jinkou/CRockerduck.hpp"
#include "jinkou/masterrobot.h"

using namespace actions;
using namespace logic;
using namespace resources;
using namespace fields;

using namespace jinkou;

CShougun* CShougun::me=NULL;

CShougun::CShougun(){
    try{
        if(me) throw new exceptions::badformatexception("SECOND SHOUGUN???");
        else {me=this; turn=-2; phase=1;}
    }
    catch(exceptions::badformatexception &ex){
        game::getgame().getout()<<ex.getmessage()<<std::endl;
    }
}

void CShougun::shougunTasks(){
    turn++;
    if(phase==1){
        if(gotei.at(0)->checkBrigade(this)) phase=2; // will be done when we have basic brigade ready;
    }
    else if(turn>=(TURNS-30)) phase=ENDGAME; // cause in short game we have just early xD
}

void CShougun::toMap(const std::map<std::pair<int, int>, fields::field*> fragment, std::pair<int, int> coords){
    /*for(std::map<std::pair<int, int>, fields::field*>::const_iterator it = fragment.begin(); it != fragment.end(); ++it) {
        char type='E';
        if(dynamic_cast<fields::resourcefield<resources::oil>*>(it->second))     type='O';
        if(dynamic_cast<fields::resourcefield<resources::metal>*>(it->second))   type='M';
        if(dynamic_cast<fields::resourcefield<resources::diamond>*>(it->second)) type='D';
        if(dynamic_cast<fields::resourcefield<resources::gold>*>(it->second))    type='G';
        lemapa[{it->first.first, it->first.second}]=type;
    }*/
    for(auto& x: fragment){
        char type='E';
        if(dynamic_cast<fields::resourcefield<resources::oil>*>(x.second))     type='O';
        if(dynamic_cast<fields::resourcefield<resources::metal>*>(x.second))   type='M';
        if(dynamic_cast<fields::resourcefield<resources::diamond>*>(x.second)) type='D';
        if(dynamic_cast<fields::resourcefield<resources::gold>*>(x.second))    type='G';
        this->lemapa.insert(std::pair<std::pair<int, int>,char>(std::pair<int, int>(x.first.first+coords.first,x.first.second+coords.second),type));
    }
}

void CShougun::toSiteMap(const std::pair<int, int> coords, TSite* site){
    this->lesitemapa.insert(std::pair<std::pair<int, int>, TSite*>(coords, site));
}

std::map<std::pair<int, int>, char> CShougun::getMap(int x, int y, int r){
    std::map<std::pair<int, int>, char> outmap;
    for (int i=x-r; i<=x+r; i++){
        for (int j=y-r; j<=y+r; j++){
            outmap.insert(std::pair<std::pair<int, int>,char>(std::pair<int, int>(i,j),lemapa[{i,j}]));
        }
    }
    return outmap;
}

void CShougun::appointTaichou(CShougun* s){
    CTaichou* t=dynamic_cast<CTaichou*>(s);
    if(t){
        try{
            t->setID(gotei.size(), me);
        }
        catch(exceptions::badformatexception &ex){
            game::getgame().getout()<<ex.getmessage()<<std::endl;
        }
        if(this->phase==2) this->phase=3;
        this->gotei.push_back(t);
    }
    else throw new exceptions::badformatexception("WHICH GODDAMN IDIOT TRIED TO CREATE THEIR OWN CPT???");
}

void CShougun::appointTaichou(CTaichou* t, CBob* requester){
    if(dynamic_cast<CBob*>(requester)){
        try{
            t->setID(gotei.size(), me);
        }
        catch(exceptions::badformatexception &ex){
            game::getgame().getout()<<ex.getmessage()<<std::endl;
        }
        if(this->phase==2) this->phase=3;
        this->gotei.push_back(t);
    }
    else throw new exceptions::badformatexception("WHICH GODDAMN IDIOT TRIED TO CREATE THEIR OWN CPT???");
}

CTaichou::CTaichou(){ //potrzebne toto??
    this->ID=-1;
    /*if(CShougun::audience(this)->getSide().first) dir=1;
    else dir=4;*/
    dir=-1;
}

CTaichou::CTaichou(int dir, TBrigade* in_b, TSite* cs):brigade(in_b),current_site(cs){
    this->ID=-1;
    this->dir=dir+1;
}

void CTaichou::initializeTheFirst(std::pair<bool,bool> x){
    if(x.first){
        if(x.second) dir=4;
        else dir=2;
    }
    else{
        if(x.second) dir=1;
        else dir=3;
    }
}

std::pair<int,int> CTaichou::newSiteCoords(){
    return this->site+dirToPair(this->dir);
}

void CTaichou::sortItStable(std::vector<std::pair<int, int>> &sort_it){
    switch(this->dir%4){
        case 0: {//maleje first
            std::stable_sort(sort_it.begin(), sort_it.end(), pathfinding_desc_second);
            std::stable_sort(sort_it.begin(), sort_it.end(), pathfinding_desc_first);
        }
        case 1: {//rośnie second
            std::stable_sort(sort_it.begin(), sort_it.end(), pathfinding_desc_first);
            std::stable_sort(sort_it.begin(), sort_it.end(), pathfinding_asc_second);
        }
        case 2: {//rośnie first
            std::stable_sort(sort_it.begin(), sort_it.end(), pathfinding_desc_second);
            std::stable_sort(sort_it.begin(), sort_it.end(), pathfinding_asc_first);
        }
        case 3: {//maleje second
            std::stable_sort(sort_it.begin(), sort_it.end(), pathfinding_desc_first);
            std::stable_sort(sort_it.begin(), sort_it.end(), pathfinding_desc_second);
        }
    }
}

void CTaichou::sortItStableByDistance(std::vector<std::pair<int, int>> &sort_it, std::pair<int, int> point){
    std::vector<int> dist=distance_to_point(sort_it, point);
    sort_it=sort_by_distance(sort_it,dist);
}

template<typename Robot> void CTaichou::sendMineCoords(std::vector<Robot*> robots, std::vector<std::pair<int,int>> in_c, bool cs){
    int miners=robots.size();
    if(miners<=0) return;
    auto ite=in_c.begin();
    auto ite2=in_c.begin()+(int)ceil(1.0*in_c.size()/miners--);
    TOrders* o;
    for(auto& x:robots){
        o=new TOrders;
        o->change_site=cs;
        o->stockpile=this->current_site->stockpile;
        o->o="mine";
        o->more_coords = new std::vector<std::pair<int,int>>;
        o->more_coords->insert(o->more_coords->begin(), ite, ite2);
        ite=ite2;
        ite2+=(int)ceil(1.0*in_c.size()/miners--);
        x->takeOrders(o, this);
    }
}

int CTaichou::leastRaba(){
    int min=this->brigade->McDuck->getRabaCount();
    for(auto& x:this->brigade->Norris) {if(x->getRabaCount()<min) min=x->getRabaCount();}
    for(auto& x:this->brigade->Rockerfeller) {if(x->getRabaCount()<min) min=x->getRabaCount();}
    for(auto& x:this->brigade->RedFox) {if(x->getRabaCount()<min) min=x->getRabaCount();}
    return min;
}

void CTaichou::takeReserves(logic::resourceset rs, logic::resourceset rs2){
    if(rs2>=0) rs=logic::min(rs, rs2);
    else rs.set<resources::diamond>(std::min(rs.get<resources::diamond>(), 800));
    TOrders* o=NULL;
    for(auto& x:this->brigade->herd){
        logic::resourceset out(0); //jakby coś pieprzło, to pewnie miałem rację -- XD
        if(+rs>0){
            out.set<resources::gold>(std::min(rs.get<resources::gold>(), 300-+out)); // in case of trouble add brackets xD
            // the Raba who takes the rest of gold violates (cenzura, Panie) the idea of taking metal and oil in 3:1:
            out.set<resources::metal>(std::min(rs.get<resources::metal>(), 300-+out-std::min(75, rs.get<resources::oil>())));
            out.set<resources::oil>(std::min(rs.get<resources::oil>(), 300-+out));
            // o kurdebele, std::min nie ogarnia trójki parametrów podanej na pałę xD a więc inaczej:
            out.set<resources::diamond>(std::min(rs.get<resources::diamond>(), 300-+out));
            rs-=out;
        }
        o=new TOrders;
        o->change_site=true;
        if(+out) o->shopping_list = new logic::resourceset(out);
        o->stockpile=this->current_site->stockpile;
        o->o="auto";
        x->takeOrders(o, this);
    }
}

CMyMiner* CTaichou::chooseTheirMaster(){
    int min=this->brigade->McDuck->getRabaCount();
    CMyMiner *newMaster=this->brigade->McDuck;
    for(auto& Gajeel:this->brigade->RedFox){
        if(Gajeel->getRabaCount()<min){
            min=Gajeel->getRabaCount();
            newMaster=Gajeel;
        }
    }
    for(auto& Chuck:this->brigade->Norris){
        if(Chuck->getRabaCount()<min){
            min=Chuck->getRabaCount();
            newMaster=Chuck;
        }
    }
    for(auto& Rockerduck:this->brigade->Rockerfeller){
        if(Rockerduck->getRabaCount()<min){
            min=Rockerduck->getRabaCount();
            newMaster=Rockerduck;
        }
    }
    return newMaster;
}

void CTaichou::checkForFreshman(){
    switch(this->brigade->freshman){
        case RABA:{
            TOrders *o = new TOrders;
            o->o="auto";
            o->master=chooseTheirMaster();
            this->brigade->herd.back()->takeOrders(o, this);
        }
        case CHUCK: {reassignDeposits<CChuck>(brigade->Norris); break;}
        case ROCKERDUCK: {reassignDeposits<CRockerduck>(brigade->Rockerfeller); break;}
        case GAJEEL: {reassignDeposits<CGajeel>(brigade->RedFox); break;}
        
        case FLINTHEART:
        case SCROOGE:
        case BOB: break;
        case NOTBOB: 
        case ZAIKO: throw new exceptions::badformatexception("WHICH GODDAMN IDIOT TRIED TO CREATE STH THAT DOESN'T EXIST???");
    }
    this->brigade->freshman=NOTBOB;
}

template<typename Robot> void CTaichou::reassignDeposits(std::vector<Robot*> robots){ //pralka i już
    std::vector<std::pair<int,int>> resource_fields;
    std::vector<std::pair<int,int>> *tmp;
    for(auto& robot:robots){
        tmp=robot->giveDeposits(this, false);
        resource_fields.insert(resource_fields.begin(), tmp->begin(), tmp->end());
        delete tmp;
    }
    int miners=robots.size();
    for(auto rit=robots.rbegin();rit!=robots.rend();rit++){
        TOrders* o = new TOrders;
        o->o="mine";
        tmp = new std::vector<std::pair<int,int>>;
        auto ite=resource_fields.begin()+(int)ceil(1.0*resource_fields.size()/miners--);
        tmp->insert(tmp->begin(), resource_fields.begin(), ite);
        resource_fields.erase(resource_fields.begin(), ite);
        o->more_coords=tmp;
        (*rit)->takeOrders(o, this);
    }
}

void CTaichou::reassignRaba(){ //10:30 28 XI - mielę
    std::vector<CMyMiner*> freshmen;
    unsigned int miner_count=1; // Scrooge;
    miner_count+=this->brigade->Norris.size();
    miner_count+=this->brigade->Rockerfeller.size();
    miner_count+=this->brigade->RedFox.size();
    unsigned int raba_count=this->brigade->herd.size();
    int r=(int)ceil(1.0*raba_count/miner_count);
    this->brigade->McDuck->newRabaCount=r;
    raba_count-=r;
    --miner_count;
    for(auto& x:this->brigade->Rockerfeller) {
        r=(int)ceil(1.0*raba_count/miner_count);
        if(r>x->getRabaCount()) freshmen.push_back(x);
        x->newRabaCount=r;
        raba_count-=r;
        --miner_count;
    }
    for(auto& x:this->brigade->RedFox) {
        r=(int)ceil(1.0*raba_count/miner_count);
        if(r>x->getRabaCount()) freshmen.push_back(x);
        x->newRabaCount=r;
        raba_count-=r;
        --miner_count;
    }
    for(auto& x:this->brigade->Norris){
        r=(int)ceil(1.0*raba_count/miner_count);
        if(r>x->getRabaCount()) freshmen.push_back(x);
        x->newRabaCount=r;
        raba_count-=r;
        --miner_count;
    }
    //jeszcze trza przydzielić; tu masz podział i rozpierdziel w komplecie
    for(auto& raba:this->brigade->herd){
        CMyMiner* master=raba->whosMyMaster(this);
        if(master->getRabaCount()>master->newRabaCount){
            master->setRabaCount(this, master->getRabaCount()-1);
            TOrders *o = new TOrders;
            o->o="auto";
            o->master=*(freshmen.begin());
            raba->takeOrders(o, this);
            (*freshmen.begin())->setRabaCount(this, (*freshmen.begin())->getRabaCount()+1);
            if((*freshmen.begin())->getRabaCount()>=(*freshmen.begin())->newRabaCount) freshmen.erase(freshmen.begin());
            //if(freshman.size()<=0) no to dupło
        }
    }
}

void CTaichou::readReports(){
    std::vector<TReport*> reports;
    try{
        reports.push_back(this->brigade->McDuck->getReport(this));
    }
    catch(exceptions::badformatexception &ex){
        game::getgame().getout()<<ex.getmessage()<<std::endl;
    }
    for(auto& robot:this->brigade->Rockerfeller){
        try{
            reports.push_back(robot->getReport(this));
        }
        catch(exceptions::badformatexception &ex){
            game::getgame().getout()<<ex.getmessage()<<std::endl;
        }
    }
    for(auto& robot:this->brigade->RedFox){
        try{
            reports.push_back(robot->getReport(this));
        }
        catch(exceptions::badformatexception &ex){
            game::getgame().getout()<<ex.getmessage()<<std::endl;
        }
    }
    for(auto& robot:this->brigade->Norris){
        try{
            reports.push_back(robot->getReport(this));
        }
        catch(exceptions::badformatexception &ex){
            game::getgame().getout()<<ex.getmessage()<<std::endl;
        }
    }
    for(auto& robot:this->brigade->herd){
        try{
            reports.push_back(robot->getReport(this));
        }
        catch(exceptions::badformatexception &ex){
            game::getgame().getout()<<ex.getmessage()<<std::endl;
        }
    }
    for(auto& report:reports){
        if(!report) continue;
        //assert(report->type==CREATE || report->type==SCOUT || report->type==BUILD); // to stable version;
        switch(report->type){
            case CREATE:    {
                TCreateReport* tmp=(TCreateReport*)(report);
                switch(tmp->what_made){ //#WARNING irrelevant warning
                    case RABA: //no break;
                    case ROCKERDUCK:{
                        if(this->brigade->herd.size()>7 &&
                           !this->brigade->Rockerfeller.empty()) this->brigade->complete=true;
                        break;
                    }
                    default: break; // to avoid that stupid warning;
                }
                if(tmp->finished || tmp->failed) reassignRaba(); //pralka
            }
            case SUPERSCOUT:
            case SCOUT:     { //scout
                TScoutReport *tmp=(TScoutReport*)(report);
                if(tmp->finished) --this->scouts;
                break;
            }
            case BUILD:     {
                TBuildReport *tmp=(TBuildReport*)(report);
                if(tmp->done) this->zaiko_ready=true;
                break;
            }
            // the rest should (shall?) never happen, so we treat them as exceptions and log:
            case MINE: game::getgame().getout()<<"SPECTACULARLY WRONG MINING REPORT!"<<std::endl; break;
            case MOVE: game::getgame().getout()<<"SPECTACULARLY WRONG MOVING REPORT!"<<std::endl; break;
            case FETCH: game::getgame().getout()<<"SPECTACULARLY WRONG FETCHING REPORT!"<<std::endl; break;
            case AUTO: game::getgame().getout()<<"SPECTACULARLY WRONG AUTO REPORT!"<<std::endl; break;
            case N: game::getgame().getout()<<"SPECTACULARLY WRONG N REPORT!"<<std::endl; break;
        }
        delete report;
    }
}

void CTaichou::sendScouting(std::vector<CMyRobot*> &scouts, bool check){
    // check parameter tells us whether we should check the number of scouts;
    TOrders *o=new TOrders;
    this->scouts=scouts.size();
    o->o="build";
    std::pair<int,int> *next=new std::pair<int, int>(this->current_site->stockpile->getSiteCenter());
    *next=*next+std::pair<int,int>(SITE_SIZE,SITE_SIZE)*dirToPair(this->dir);
    // they actually scout at the size of 4 xD
    o->coords=next;
    if(check && scouts.size()==1){
        o->change_site=true; // Flintheart is alone - this flag will tell him to perform superScout;
        scouts.at(0)->takeOrders(o, this);
        scouts.erase(scouts.begin());
        return;
    }
    scouts.at(0)->takeOrders(o, this);
    scouts.erase(scouts.begin());
    if(check && scouts.size()==1){
        o = new TOrders;
        o->o="superscout"; // Flintheart has only one Raba who will perform superScout;
        o->coords=new std::pair<int,int>(*next);
        scouts.at(0)->takeOrders(o, this);
        scouts.erase(scouts.begin());
        return;
    }
    // we don't need to check when we have two Raba here:
    short positioner=2;
    while(!scouts.empty()){
        o = new TOrders;
        o->o="scout";
        short one=positioner/2;
        short two=Signumoid(positioner%2);
        short three=this->dir+two;
        o->coords=new std::pair<int,int>(this->current_site->stockpile->getSiteCenter()+
            std::pair<int,int>(one*3, one*3)*dirToPair(three));   //współrzędne, scout dla Flintheart'a, potwierdzenie zakończenia scoutu
        o->coords2=new std::pair<int,int>(this->current_site->stockpile->getSiteCenter()+
            std::pair<int,int>(one*3, one*3)*dirToPair(three)+
            std::pair<int,int>(SITE_SIZE,SITE_SIZE)*dirToPair(this->dir));
        scouts.at(0)->takeOrders(o, this);
        scouts.erase(scouts.begin());
        ++positioner;
    }
}

TPlan CTaichou::planSomething(){
    std::map<std::pair<int,int>, char> site_map;
    try{
        std::pair<int,int> crd=current_site->stockpile->getSiteCenter();
        site_map=CShougun::audience(this)->getMap(crd.first, crd.second, SITE_SIZE);
    }
    catch(exceptions::badformatexception &ex){
        game::getgame().getout()<<ex.getmessage()<<std::endl;
    }
    TPlan the_plan;
    for(auto& x: site_map){
        switch(x.second){
            case 'O': the_plan.o.push_back(x.first);
            case 'M': the_plan.m.push_back(x.first);
            case 'D': the_plan.d.push_back(x.first);
            case 'G': the_plan.g.push_back(x.first);
        }
    }
    sortItStableByDistance(the_plan.o, this->current_site->stockpile->getCoords());
    sortItStableByDistance(the_plan.m, this->current_site->stockpile->getCoords());
    sortItStable(the_plan.d);
    sortItStable(the_plan.g);
    if(this->brigade->Rockerfeller.empty()) the_plan.g.insert(the_plan.g.begin(),the_plan.o.front());
    return the_plan;
}

bool addRaba(logic::resourceset zaiko_res, logic::resourceset& brigade_cost, unsigned int &U, std::vector<ERobot>* tm){
    if(zaiko_res>brigade_cost+getCost(RABA)){
        tm->push_back(RABA);
        brigade_cost+=getCost(RABA);
        U++;
        return true;
    }
    else return false;
}

void CTaichou::brigadeExpansion(){
    // how to expand our brigade:
    int p=CShougun::audience(this)->getPhase();
    logic::resourceset brigade_cost(0); //do parametru to potem;
    logic::resourceset tmp=this->current_site->stockpile->getloadedresources()-brigade_cost;
    std::vector<ERobot>* tm=new std::vector<ERobot>();
    TOrders* o=NULL;
    unsigned r=this->brigade->Rockerfeller.size()+this->brigade->RedFox.size()+this->brigade->Norris.size()+1;
    unsigned U=this->brigade->herd.size(); // not only iterator, but counter as well;
    while(U<(3*r)){
        if(!addRaba(tmp, brigade_cost, U, tm)) break;
    }
    if((1.0*this->plan.m.size()/this->brigade->RedFox.size())>1.5){
        if(tmp>brigade_cost+getCost(GAJEEL)){
            tm->push_back(GAJEEL);
            brigade_cost+=getCost(GAJEEL);
            r++;
            addRaba(tmp, brigade_cost, U, tm);
            addRaba(tmp, brigade_cost, U, tm);
        }
    }
    if((1.0*this->plan.o.size()/(this->brigade->Rockerfeller.size()+1))>2.0){
        if(tmp>brigade_cost+getCost(ROCKERDUCK)){
            tm->push_back(ROCKERDUCK);
            brigade_cost+=getCost(ROCKERDUCK);
            r++;
            addRaba(tmp, brigade_cost, U, tm);
            addRaba(tmp, brigade_cost, U, tm);
        }
    }
    if((1.0*this->plan.d.size()/this->brigade->Norris.size())>2.0){
        if(tmp>brigade_cost+getCost(CHUCK)){
            tm->push_back(CHUCK);
            brigade_cost+=getCost(CHUCK);
            r++;
            addRaba(tmp, brigade_cost, U, tm);
            addRaba(tmp, brigade_cost, U, tm);
        }
    }
    // more Raba:
    while(U<(5*r)){
        if(!addRaba(tmp, brigade_cost, U, tm)) break;
    }
    if(!tm->size()){delete tm; tm=NULL;}
    if(this->ID<2 && this->brigade->complete){
        // building brigade, but complete (incomplete changes nothing):
        if(tm&&tm->size()>3){
            o = new TOrders;
            o->o="create";
            o->make_it = tm;
            this->brigade->builder->takeOrders(o, this);
        }
        else{
            delete tm;
            tm=NULL;
            int I=0;
            ERobot next=BRIGADE_TEMPLATE.at(I);
            brigade_cost+=getCost(ZAIKO);
            if(p<ENDGAME){
                while(tmp>brigade_cost+getCost(next)){
                    brigade_cost+=getCost(next);
                    ++I;
                    next=BRIGADE_TEMPLATE.at(I);
                }
                if(I>2) ++I;
                next=BRIGADE_TEMPLATE.at(I);
                while(tmp>brigade_cost+getCost(next)){
                    brigade_cost+=getCost(next);
                    ++I;
                    next=BRIGADE_TEMPLATE.at(I);
                }
                if(I>2){
                    o = new TOrders;
                    o->o="create";
                    o->make_it = new std::vector<ERobot>(BRIGADE_TEMPLATE);
                    this->brigade->builder->takeOrders(o, this);
                }
            }
        }
        takeReserves(tmp-brigade_cost); // this will also give them orders;
    }
    if(this->ID>1){ // not building brigade:
        if(tm->size()){
            o = new TOrders;
            o->o="create";
            o->make_it = tm;
            this->brigade->builder->takeOrders(o, this);
        }
        else{
            delete tm;
            tm=NULL;
        }
        logic::resourceset shopping_list(600, 600, 0, 600);
        takeReserves(tmp-brigade_cost, shopping_list);
    }
}

void CTaichou::changeSite(){
    // we're at least in mid game;
    this->gold_mined=false; this->diamond_mined=false;
    this->zaiko_ready=false;
    this->site=newSiteCoords();
    try{
        this->current_site=CShougun::audience(this)->getSiteMap()[this->site];
    }
    catch(exceptions::badformatexception &ex){
        game::getgame().getout()<<ex.getmessage()<<std::endl;
    }
    CZaiko *stockpile=this->current_site->stockpile;
    this->plan=this->planSomething(); //TU MIELISZ! PATRZAJ SIĘ ŚLEPAKU!
    // give orders and deposits to everybody:
    TOrders* o = new TOrders;
    o->change_site=true;
    o->stockpile=stockpile;
    o->o="mine";
    o->more_coords=new std::vector<std::pair<int,int>>(this->plan.g);
    this->brigade->McDuck->takeOrders(o,this);
    sendMineCoords(this->brigade->Norris, this->plan.d);
    sendMineCoords(this->brigade->RedFox, this->plan.m);
    sendMineCoords(this->brigade->Rockerfeller, this->plan.o);
    brigadeExpansion();
}

void CTaichou::manageTeam(){
    bool out=true;
    TOrders* o;
    readReports();
    try{
        checkForFreshman();
    }
    catch(exceptions::badformatexception &ex){
        game::getgame().getout()<<ex.getmessage()<<std::endl;
    }
    std::vector<CMyRobot*> scouting;
    if(this->current_site->cpt!=this){ // so I'm newly built:
        if(!scouts && zaiko_ready) changeSite();
        else{
            if(this->brigade->builder && this->brigade->Glomgold &&
               !this->brigade->McDuck->isBusy() && !this->brigade->builder->isBusy() &&
               !this->brigade->Glomgold->isBusy()){
                
                scouting.push_back(this->brigade->Glomgold);
                scouting.push_back(this->brigade->McDuck);
                scouting.push_back(this->brigade->builder);
                sendScouting(scouting);
            }
        }
        return; //zmienić VVV w else?
    }
    
    //if(this->brigade->complete){
    if(this->brigade->McDuck->isBusy(true)) out=false;
    for(auto& x:this->brigade->Norris){
        // here x is a robot, a Chuck:
        if(x->isBusy(true)) {out=false;}
        else if(!this->diamond_mined){
            diamond_mined=true;
            
            scouting.push_back(this->brigade->Glomgold);
            
            int mules=0;
            for(auto& mule:this->brigade->herd){
                // what if he don't have 2 Raba?
                if(mule->whosMyMaster(this)==(CMyRobot*)this->brigade->Glomgold){
                    scouting.push_back(mule);
                    ++mules;
                }
                if(mules==2) break;
            }
            
            if(x!=this->brigade->Glomgold){
                o = new TOrders;
                o->o="mine";
                o->more_coords=this->brigade->Glomgold->giveDeposits(this); // he will take the rest of Flintheart's deposits;
                x->takeOrders(o,this);
            }
            // if Flintheart, he have mined his deposits;
            sendScouting(scouting, true); //we have to check how many of them there are;
            
            break;
        }
    }
    if(out && zaiko_ready && !scouts) changeSite();
    else{ // we don't change site:
        // build orders for every robot:
        //standard orders for everybody:
        if(!this->brigade->McDuck->isBusy(true) && !gold_mined) {
            gold_mined=true;
            std::vector<std::pair<int,int>> oil_fields;
            std::vector<std::pair<int,int>> *tmp;
            for(auto& x:this->brigade->Rockerfeller){
                tmp=x->giveDeposits(this, false);
                oil_fields.insert(oil_fields.begin(), tmp->begin(), tmp->end());
                delete tmp;
            }
            int miners=this->brigade->Rockerfeller.size()+1;
            o = new TOrders;
            o->o="mine";
            tmp = new std::vector<std::pair<int,int>>;
            auto ite=oil_fields.begin()+(int)ceil(1.0*oil_fields.size()/miners--);
            tmp->insert(tmp->begin(), oil_fields.begin(), ite);
            oil_fields.erase(oil_fields.begin(), ite);
            o->more_coords=tmp;
            this->brigade->McDuck->takeOrders(o, this);
            for(auto& x:this->brigade->Rockerfeller){
                o = new TOrders;
                o->o="mine";
                tmp = new std::vector<std::pair<int,int>>;
                ite=oil_fields.begin()+(int)ceil(1.0*oil_fields.size()/miners--);
                tmp->insert(tmp->begin(), oil_fields.begin(), ite);
                oil_fields.erase(oil_fields.begin(), ite);
                o->more_coords=tmp;
                x->takeOrders(o, this);
            }
        }
    }
    
    
    if(o) delete o; o=NULL;
}

void CTaichou::setID(int ID, CShougun* requester){
    if(dynamic_cast<CShougun*>(requester)) this->ID=ID;
    else throw new exceptions::badformatexception("WHICH GODDAMN IDIOT WANTED TO CHANGE MY ID???");
}

void CTaichou::setBrigade(TBrigade* brigade, CMyRobot* requester){
    if((dynamic_cast<CShougun*>(requester))||(dynamic_cast<CBob*>(requester))) this->brigade=brigade;
    else throw new exceptions::badformatexception("WHICH GODDAMN IDIOT WANTED TO SET MY BRIGADE???");
}

TSite* CTaichou::getNewSite(CFlintheart* requester){
    if(dynamic_cast<CFlintheart*>(requester)==NULL)
        throw new exceptions::badformatexception("WHICH GODDAMN IDIOT WANTED TO CHANGE MY SITE???");
    TSite *s=new TSite;
    std::pair<int,int> crd=newSiteCoords();
    try{
        CShougun::audience(this)->toSiteMap(crd, s);
    }
    catch(exceptions::badformatexception &ex){
        game::getgame().getout()<<ex.getmessage()<<std::endl;
    }
    return s;
}

TBrigade* CTaichou::getBrigade(CBob* requester){
    if(dynamic_cast<CBob*>(requester)){
        return this->brigade;
    }
    else throw new exceptions::badformatexception("WHICH GODDAMN IDIOT WANTED TO KNOW MY BRIGADE???");
}

bool CTaichou::checkBrigade(CShougun* requester){
    if(dynamic_cast<CShougun*>(requester)){
        return (this->brigade && this->brigade->builder && this->brigade->Glomgold);
    }
    else{
        delete requester; // FUN!!! :D
        throw new exceptions::badformatexception("WHO THE HELL PRETENDS TO BE MY SHOUGUN???");
    }
}

void CMyRobot::takeOrders(TOrders *orders, CTaichou *requester){
    if(dynamic_cast<CTaichou*>(requester)){
        if(this->orders) delete this->orders;
        this->orders=orders;
    }
    else throw new exceptions::badformatexception("WHICH GODDAMN IDIOT TRIED TO GIVE ME ORDERS???");
}

TReport* CMyRobot::getReport(CTaichou* requester){
    if(dynamic_cast<CTaichou*>(requester)){
        TReport* tmp=this->report;
        this->report=NULL;
        return tmp;
    }
    else throw new exceptions::badformatexception("WHICH GODDAMN IDIOT TRIED TO STEAL MY REPORT???");
}