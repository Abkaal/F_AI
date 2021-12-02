#include "jinkou/CScrooge.hpp"

using namespace logic;
using namespace resources;
using namespace fields;

using namespace jinkou;

actions::action* CScrooge::chooseAction(const std::vector<actions::action *> &possibleactions) {
    manageTeam();
    //formGrandReport();
    try{
        CShougun::audience(dynamic_cast<CMyRobot*>(this))->toMap(getSurroundings(), getCoords());
    }
    catch(exceptions::badformatexception &ex){
        game::getgame().getout()<<ex.getmessage()<<std::endl;
    }
    this->changeMode();
    return this->defaultBehaviour(possibleactions);
}