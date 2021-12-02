#ifndef CMYROBOT_HPP
#define CMYROBOT_HPP

#include "actions/action.h"
#include "actions/mine.h"
#include "actions/move.h"
#include "actions/create.h"
#include "actions/build.h"
#include "actions/transfer.h"

#include "logic/game.h"

#include "robots/robot.h"
#include "robots/transporter.h"

#include "fields/resourcefield.h"

#include "jinkou/Structures.hpp"
#include "jinkou/CZaiko.hpp"

namespace jinkou{
    class CMyRobot:virtual public robots::transporter{ // transporter inherits from robot
        actions::action* fuelUp(const std::vector<actions::action *> &possibleactions); // will be invoked once per robot;
        std::pair<int, int> coords;
        protected:
        std::pair<int, int> *scout_from, *scout_to;
        std::pair<int, int> *go_to;
        TScoutingData* sd;
        bool busy;
        ECodes currentMode;
        CZaiko *bank;
        logic::resourceset *shopping_list;
        CZaiko *quantumstockpile;
        TOrders *orders;
        TReport *report;
        bool override;
        virtual void changeMode();
        actions::action* load(const std::vector<actions::action *> &possibleactions);
        actions::action* drop(const std::vector<actions::action *> &possibleactions, int fuel=50);
        actions::action* scout(const std::vector<actions::action *> &possibleactions);
        actions::action* superScout(const std::vector<actions::action *> &possibleactions);
        actions::action* move(const std::vector<actions::action *> &possibleactions);
        actions::action* move(const std::vector<actions::action *> &possibleactions, std::pair<int, int> acoords);
        CMyRobot(std::pair<int, int> in_c):
                        coords(in_c),
                        scout_from(NULL),scout_to(NULL),go_to(NULL),sd(NULL),
                        busy(false),currentMode(N),
                        bank(NULL),shopping_list(NULL),quantumstockpile(NULL),orders(NULL),
                        report(NULL),
                        override(false)
                        {};
        bool shouldITakeOrders();
        public:
        std::pair<int, int> getCoords(){return coords;}
        CMyRobot() =delete;
        std::map<std::pair<int, int>, fields::field *> getSurroundings(){return this->getmyposition()->getsurroundings(this);}
        virtual actions::action* performaction(const std::vector<actions::action *> &possibleactions);
        virtual actions::action* chooseAction(const std::vector<actions::action *> &possibleactions) =0;
        virtual ~CMyRobot(){
            if(orders) delete orders; orders=NULL;
            if(go_to) delete go_to; go_to=NULL;
            if(scout_from) delete scout_from; scout_from=NULL;
            if(scout_to) delete scout_to; scout_to=NULL;
        };
        bool isBusy(){return busy;}
        TReport* getReport(CTaichou* requester); // implemented in Shikikan.cpp
        void takeOrders(TOrders *orders, CTaichou *requester); // same as above;
        std::pair<int, int> whereAreWeGoingFromHere();
        std::pair<int, int> whereAreWeGoingFromHere(std::pair<int,int> coords);
    };
}
#endif