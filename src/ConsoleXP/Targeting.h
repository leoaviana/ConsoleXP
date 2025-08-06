#pragma once
#include <cstdint> 
#include <vector>
#include <string>

struct mob_entity {
    uint64_t GUID;
    float distance;
    int targetMark;
};

typedef uint64_t(*MOB_SELECTFUNCTION)(uint64_t current, std::vector<mob_entity>& list);
typedef uint64_t(*MOB_SELECTFUNCTION_WITH_MARK)(uint64_t current, std::vector<mob_entity>& list, const std::vector<int>& markPriority);

namespace Targeting
{

    // Factor which define targeting range cone in front of camera. Minimum 2 is exactly same as FoV. Default is 2.2. Bigger value narrows the cone.
    extern float targetingRangeCone;
    extern int virtualMouseX;
    extern int virtualMouseY;

    uint32_t CollectInteractKeyTarget();
    uint32_t CollectInteractMouseOverTarget();
    // Return true when found a target
    bool TargetNearestEnemy(float distanceLimit);
    bool TargetNearestEnemyFacing(float distanceLimit, uint64_t* outGuid, float targetingCone = 0.30f);
    bool TargetWorldBoss(float distanceLimit);
    bool TargetEnemyConsideringDistance(MOB_SELECTFUNCTION selectFunction);
    bool TargetEnemyInCycle(MOB_SELECTFUNCTION selectFunction);
    bool TargetMarkedEnemyInCycle(MOB_SELECTFUNCTION_WITH_MARK selectFunction, std::string priority);

    // Select mobs sorted by GUID
    uint64_t SelectNext(uint64_t current, std::vector<struct mob_entity>& list);
    uint64_t SelectPrevious(uint64_t current, std::vector<struct mob_entity>& list);

    // Select mobs with mark;
    uint64_t SelectNextMark(const uint64_t current, std::vector<struct mob_entity>& list, const std::vector<int>& priority);
    uint64_t SelectPreviousMark(const uint64_t current, std::vector<struct mob_entity>& list, const std::vector<int>& priority);
}
