#ifndef CROCKERDUCK_HPP
#define CROCKERDUCK_HPP

#include "actions/action.h"

#include "robots/robot.h"
#include "robots/builder.h"
#include "robots/creator.h"
#include "robots/transporter.h"

#include "resources/diamond.h"
#include "resources/gold.h"
#include "resources/metal.h"
#include "resources/oil.h"

#include "jinkou/CMyMiner.hpp"

// cena:
// ropa    - 425  *0.2
// metal   - 400  *0.5
// diamond - 0    *0.8
// złoto   - 200  *1.0
// ładowność:
// 400
// zawód wyuczony:
// miliarder za dychę

namespace jinkou {
    
    class CRockerduck: 
        virtual public robots::transporter,
        virtual public robots::concreteminer<resources::oil>,
        virtual public CMyMiner{
    public:
        virtual actions::action* chooseAction(const std::vector<actions::action *> &possibleactions);
        CRockerduck(std::pair<int, int> coords):CMyRobot(coords),CMyMiner(coords){};
    };
        
    
}

#endif
