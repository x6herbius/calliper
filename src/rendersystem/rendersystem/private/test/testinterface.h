#ifndef TESTINTERFACE_H
#define TESTINTERFACE_H

#include "rendersystem_global.h"
#include "rendersystem/interfaces/itestinterface.h"

class TestInterface : public RenderSystem::ITestInterface
{
public:
    TestInterface();

    QString giveMeAString() override;
};

#endif // TESTINTERFACE_H