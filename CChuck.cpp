#include "jinkou/CChuck.hpp"
#include "jinkou/Shikikan.hpp"

using namespace logic;
using namespace resources;
using namespace fields;

using namespace jinkou;

actions::action* CChuck::chooseAction(const std::vector<actions::action *> &possibleactions) {
    try{
        CShougun::audience(dynamic_cast<CMyRobot*>(this))->toMap(getSurroundings(), getCoords());
    }
    catch(exceptions::badformatexception &ex){
        game::getgame().getout()<<ex.getmessage()<<std::endl;
    }
    changeMode();
    return defaultBehaviour(possibleactions);
}