#ifndef SHIKIKAN_HPP
#define SHIKIKAN_HPP

#include<map>

#include "actions/action.h"

#include "robots/robot.h"
#include "robots/builder.h"
#include "robots/creator.h"
#include "robots/transporter.h"

#include "resources/diamond.h"
#include "resources/gold.h"
#include "resources/metal.h"
#include "resources/oil.h"

#include "exceptions/badformatexception.h"

#include "jinkou/Structures.hpp"
#include "jinkou/CMyRobot.hpp"

namespace jinkou {
    
// cena:
// bezcenny
// ładowność:
// 0
// famous for:
// lets emperor be "emperor"
    
    class CShougun{
        static CShougun *me;
        std::pair<bool, bool> side;
        std::map<std::pair<int, int>, char> lemapa;
        std::map<std::pair<int, int>, TSite*> lesitemapa;
        //std::vector<TGrandReport> reports;
        std::vector<CTaichou*> gotei;
        //int whoCanHelp(CTaichou *helpee, ERobot er);
        int turn;
        protected:
        bool right;
        int phase;
        bool knowHeIsTaichou;
        //void readReports();
        void shougunTasks();
        void remember();
        void setSide(std::pair<bool,bool> in_side){side.first=in_side.first;side.second=in_side.second;};
        public:
        void toMap(const std::map<std::pair<int, int>, fields::field*> fragment, std::pair<int, int> coords);
        void depositRunOut(std::pair<int, int> coords){lemapa[coords]='E';};
        void toSiteMap(const std::pair<int, int> coords, TSite* site);
        TSite* getSiteMap(std::pair<int, int> coords){return lesitemapa.at(coords);}
        std::map<std::pair<int, int>, TSite*> getSiteMap(){return lesitemapa;}
        std::map<std::pair<int, int>, char> getMap(){return lemapa;}
        std::map<std::pair<int, int>, char> getMap(int x, int y, int r=0);
        char getMap(std::pair<int, int> coords){return lemapa.at(coords);}
        void appointTaichou(CShougun* s);
        void appointTaichou(CTaichou* t, CBob* requester);
        int getPhase(){return phase;}
        //void report(TGrandReport report);
        std::pair<bool, bool> getSide(){return side;}
        static CShougun* audience(CMyRobot* requester){
            //if((dynamic_cast<CMyRobot*>(requester))&&(team==requester->getteam())) return me;
            if(dynamic_cast<CMyRobot*>(requester)) return me;
            else{
                delete requester; //FUN!
                throw new exceptions::badformatexception("BEHEAD THE FOE WHO TRIED TO GET AN AUDIENCE!!!");
            }
        };
        static CShougun* audience(CTaichou* requester){
            //if((dynamic_cast<CMyRobot*>(requester))&&(team==requester->getteam())) return me;
            return me;
        };
        CShougun();
        virtual ~CShougun(){};
        virtual actions::action* chooseAction(const std::vector<actions::action *> &possibleactions) =0; // just to make Shougun abstract;
    };
    
// cena:
// bezcenny
// ładowność:
// 0
// zdolności specjalne:
// bankai (no... poza jedenastym)
    
    class CTaichou{
        int ID;
        bool gold_mined=false, diamond_mined=false;
        std::pair<int, int> site;
        short dir; // 1=>up and then clock-wise
        bool zaiko_ready=false;
        //TGrandOrders grandOrders;
        TBrigade *brigade=NULL;
        std::pair<int,int> newSiteCoords();
        bool checkBrigadeCost();
        void changeSite();
        void takeReserves(logic::resourceset rs, logic::resourceset rs2=logic::resourceset(-1));
        void sendScouting(std::vector<CMyRobot*>& scouts, bool check=false);
        void sortItStable(std::vector<std::pair<int, int>> &sort_it);
        void sortItStableByDistance(std::vector<std::pair<int, int>> &sort_it, std::pair<int, int> point);
        template<typename Robot> void sendMineCoords(std::vector<Robot*> robots, std::vector<std::pair<int,int>> in_c, bool cs=true);
        void checkForFreshman();
        CMyMiner* chooseTheirMaster();
        int leastRaba();
        template<typename Robot> void reassignDeposits(std::vector<Robot*> robots);
        void reassignRaba();
    protected:
        TPlan plan;
        TSite *current_site=NULL, *next_site=NULL;
        short scouts=0;
        void manageTeam();
        void readReports();
        //void formGrandReport();
        void brigadeExpansion();
        TPlan planSomething();
    public:
        std::pair<int, int> siteCoords(){return site;}
        short getDir(){return dir;}
        void setDir(short in_d, CShougun* requester){
            if(dynamic_cast<CShougun*>(requester)) this->dir=in_d;
            else throw new exceptions::badformatexception("WHICH GODDAMN IDIOT WANTED TO CHANGE MY DIRECTION???");
        }
        void setID(int ID, CShougun* requester);
        int getID(){return ID;}
        TBrigade* getBrigade(CBob* requester);
        bool checkBrigade(CShougun* requester);
        void setBrigade(TBrigade* brigade, CMyRobot* requester);
        void initializeTheFirst(std::pair<bool,bool> x);
        //bool canYouHelp(ERobot er);
        TSite* getNewSite(CFlintheart *requester);
        CTaichou();
        CTaichou(int dir, TBrigade* in_b, TSite* cs);
        //void takeGrandOrders(TGrandOrders go, CShougun* requester);
        virtual actions::action* chooseAction(const std::vector<actions::action *> &possibleactions) =0; // just to make Taichou abstract;
        virtual ~CTaichou(){
            if(current_site) delete current_site; current_site=NULL;
            if(next_site) delete next_site; next_site=NULL;
            if(brigade) delete brigade; brigade=NULL;
        }
        friend class masterrobot; //  to po to, żeby master zawsze miał nad sobą kontrolę
    };
        
    
}

#endif