#ifndef CMYMINER_HPP
#define CMYMINER_HPP

#include "actions/action.h"
#include "actions/mine.h"
#include "actions/move.h"

#include "robots/robot.h"

#include "fields/resourcefield.h"

#include "resources/oil.h"

#include "CMyRobot.hpp"
#include "Structures.hpp"
#include "Shikikan.hpp"

namespace jinkou{
    class CMyMiner:virtual public CMyRobot{
        bool continueMining; // set to true when a miner needs to get to the place they should mine;
        int rabaCount;
        actions::action* mine(const std::vector<actions::action *> &possibleactions);
        protected:
        std::vector<std::pair<int,int>>* giveDeposits();
        void autoPilot();
        std::vector<std::pair<int,int>> *deposits;
        virtual void changeMode();
        virtual actions::action* defaultBehaviour(const std::vector<actions::action *> &possibleactions);
        public:
        int newRabaCount;
        void setRabaCount(CTaichou* requester, int in_rc){
            if(dynamic_cast<CTaichou*>(requester)) this->rabaCount=in_rc;
            else throw new exceptions::badformatexception("WHICH GODDAMN IDIOT WANTED TO CHANGE MY ID???");
        };
        int getRabaCount(){return rabaCount;};
        bool isBusy(bool mining=false){
            if(mining) return (this->busy || this->continueMining);
            else return this->busy;
        }
        CMyMiner() =delete;
        CMyMiner(std::pair<int, int> in_c):
                            CMyRobot(in_c),
                            continueMining(false),rabaCount(0),deposits(NULL){
                                //this->report=new TReport;
                            }
        std::vector<std::pair<int,int>>* giveDeposits(CTaichou* requester, bool current=true);
        virtual ~CMyMiner(){if(deposits) delete deposits; deposits=NULL;}
    };
}
#endif