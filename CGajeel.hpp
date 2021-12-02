#ifndef CGAJEEL_HPP
#define CGAJEEL_HPP

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
// ropa    - 225  *0.2
// metal   - 600  *0.5
// diamond - 0    *0.8
// złoto   - 200  *1.0
// ładowność:
// 400
// informacje dodatkowe:
// jest na diecie

namespace jinkou {
    
    class CGajeel: 
        virtual public robots::transporter,
        virtual public robots::concreteminer<resources::metal>,
        virtual public CMyMiner{
    public:
        virtual actions::action* chooseAction(const std::vector<actions::action *> &possibleactions);
        CGajeel(std::pair<int, int> coords):CMyRobot(coords),CMyMiner(coords){};
    };
        
    
}

#endif
