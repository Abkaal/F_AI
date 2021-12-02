#ifndef CRABA_HPP
#define CRABA_HPP

#include "actions/action.h"

#include "robots/robot.h"
#include "robots/transporter.h"

#include "resources/diamond.h"
#include "resources/gold.h"
#include "resources/metal.h"
#include "resources/oil.h"

#include "jinkou/CMyMiner.hpp"
#include "jinkou/CZaiko.hpp"

// cena:
// ropa    - 225  *0.2
// metal   - 200  *0.5
// diamond - 0    *0.8
// złoto   - 0    *1.0
// ładowność:
// 400
// pomyślunek:
// NULL

namespace jinkou {
    
    class CRaba: 
        virtual public robots::transporter,
        virtual public CMyRobot {
        CMyMiner *myMaster=NULL;
        //actions::action* fetch(const std::vector<actions::action *> &possibleactions);
        actions::action* autoPilot(const std::vector<actions::action *> &possibleactions);
        void changeMode();
    public:
        CMyMiner* whosMyMaster(CTaichou *requester){return myMaster;} // throw sth!!!
        virtual actions::action* chooseAction(const std::vector<actions::action *> &possibleactions);
        CRaba(std::pair<int, int> coords):
                        CMyRobot(coords)
                        {};
        virtual ~CRaba(){
            if(shopping_list) delete shopping_list; shopping_list=NULL;
        };
    };
        
    
}

#endif
