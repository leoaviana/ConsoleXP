#include "pch.h"
#include "Targeting.h"
#include "Game.h" 

#include <algorithm>
#include <limits>

namespace Targeting
{
    extern float targetingRangeCone = 2.2f;
    extern int virtualMouseX = 0;
    extern int virtualMouseY = 0;


    uint32_t CollectInteractKeyTarget()
    {
        if (!Game::IsInWorld()) return 0;

        uint32_t objectsBase = *reinterpret_cast<uint32_t*>(Offsets::VISIBLE_OBJECTS);
        uint32_t objects = *reinterpret_cast<uint32_t*>(objectsBase + 0x2ED0);
        uint32_t currentObject = *reinterpret_cast<uint32_t*>(objects + 0xAC);

        uint64_t playerGUID = *reinterpret_cast<uint64_t*>(objects + 0xC0);
        uint32_t player = Game::GetObjectPointer(playerGUID);

        C3Vector pPos = Game::GetUnitPosition(player);
        C3Vector oPos;

        uint32_t candidate = 0;

        float bestDistance = 1000.0f;

        while (currentObject != 0 && (currentObject & 1) == 0)
        {
            uint64_t guid = *reinterpret_cast<uint64_t*>(currentObject + 0x30);
            uint32_t pointer = Game::GetObjectPointer(guid);
            uint32_t type = *reinterpret_cast<uint32_t*>(pointer + 0x14);

            // Ignore summoned units from players
            uint64_t summonedByGUID = *reinterpret_cast<uint64_t*>(*reinterpret_cast<uint32_t*>(pointer + 0x8) + 0x30);
            uint32_t summonedBy = Game::GetObjectPointer(summonedByGUID);
            if (summonedByGUID && summonedBy) {
                uint32_t ownerType = *reinterpret_cast<uint32_t*>(summonedBy + 0x14);
                if (ownerType == ObjectType::PLAYER) {
                    currentObject = *reinterpret_cast<uint32_t*>(currentObject + 0x3C);
                    continue;
                }
            }

            // Get position
            if (type == ObjectType::UNIT)
                oPos = Game::GetUnitPosition(currentObject);
            else if (type == ObjectType::GAMEOBJECT)
                oPos = Game::GetObjectPosition(currentObject);
            else {
                currentObject = *reinterpret_cast<uint32_t*>(currentObject + 0x3C);
                continue;
            }

            float distance = distance3D(oPos, pPos);
            if ((distance <= 5.0 && distance < bestDistance) && Game::IsFacingMelee(player, currentObject)) {
                if (type == ObjectType::UNIT) {
                    if (!Game::IsUnitInteractable(currentObject) || !Game::IsUnitInLosTo(player, currentObject)) {
                        currentObject = *reinterpret_cast<uint32_t*>(currentObject + 0x3C);
                        continue;
                    }

                    if (Game::GetUnitHealth(currentObject) == 0 &&
                        (Game::IsUnitLootable(currentObject) || Game::IsUnitSkinnable(currentObject))) {
                        bestDistance = distance;
                        candidate = currentObject;
                    }
                    else if (Game::GetUnitHealth(currentObject) > 0) {
                        bestDistance = distance;
                        candidate = currentObject;
                    }
                }
                else if (type == ObjectType::GAMEOBJECT) {
                    if (!Game::IsGameObjectInteractable(currentObject) ||
                        (!Game::IsUnitInLosTo(player, currentObject) &&
                            !Game::IsUnitInLosTo(player, currentObject, 0x00000011)))
                    {
                        currentObject = *reinterpret_cast<uint32_t*>(currentObject + 0x3C);
                        continue;
                    }

                    bestDistance = distance;
                    candidate = currentObject;
                }
            }

            currentObject = *reinterpret_cast<uint32_t*>(currentObject + 0x3C);
        }

        return candidate;
    }


    uint32_t CollectInteractMouseOverTarget()
    {
        if (!Game::IsInWorld()) return 0;

        uint32_t objectsBase = *reinterpret_cast<uint32_t*>(Offsets::VISIBLE_OBJECTS);
        uint32_t objects = *reinterpret_cast<uint32_t*>(objectsBase + 0x2ED0);
        uint32_t current = *reinterpret_cast<uint32_t*>(objects + 0xAC);

        int screenW, screenH;
        Game::GetScreenSize(&screenW, &screenH);

        int centerX = (screenW / 2) + virtualMouseX;
        int centerY = (screenH / 2) - virtualMouseY;

        uint64_t playerGUID = *reinterpret_cast<uint64_t*>(objects + 0xC0);
        uint32_t player = Game::GetObjectPointer(playerGUID);
        C3Vector pPos = Game::GetUnitPosition(player);

        uint32_t bestTarget = 0;

        float radius = 0.5f;
        float height = 2.0f;

        while (current && !(current & 1))
        {
            uint32_t type = *reinterpret_cast<uint32_t*>(current + 0x14);
            if ((type == ObjectType::UNIT || type == ObjectType::GAMEOBJECT))
            {
                C3Vector worldPos = Game::GetObjectPosition(current);
                float descriptorScale = *(float*)(current + 0x9C);
                float baseScale = *(float*)(current + 0x98);
                float trueScale = descriptorScale * baseScale;

                C3Vector oPos = Game::GetObjectPosition(current);
                uint64_t summonedByGUID = *reinterpret_cast<uint64_t*>(*reinterpret_cast<uint32_t*>(current + 0x8) + 0x30);
                uint32_t summonedBy = Game::GetObjectPointer(summonedByGUID);
                if (summonedByGUID != 0 && summonedBy != 0)
                {
                    uint32_t owner = *reinterpret_cast<uint32_t*>(summonedBy + 0x14);
                    if (owner == ObjectType::PLAYER)
                    {
                        current = *reinterpret_cast<uint32_t*>(current + 0x3C);
                        continue;
                    }
                }

                if (!Game::IsFacingMelee(player, current) || distance3D(oPos, pPos) > 5.0f)
                {
                    current = *reinterpret_cast<uint32_t*>(current + 0x3C);
                    continue;
                }

                if (type == ObjectType::GAMEOBJECT)
                {
                    if (!Game::IsUnitInLosTo(player, current))
                    {
                        if (!Game::IsUnitInLosTo(player, current, 0x00000011))
                        {
                            current = *reinterpret_cast<uint32_t*>(current + 0x3C);
                            continue;
                        }
                    }

                    if (!Game::IsGameObjectInteractable(current))
                    {
                        current = *reinterpret_cast<uint32_t*>(current + 0x3C);
                        continue;
                    }

                    uintptr_t model = *(uintptr_t*)(current + 0xB4);
                    if (model)
                    {
                        uintptr_t renderInfo = *(uintptr_t*)(model + 0x2C);
                        if (renderInfo)
                        {
                            uintptr_t boundsStruct = *(uintptr_t*)(renderInfo + 0x150);
                            if (boundsStruct)
                            {
                                radius = *(float*)(boundsStruct + 0xB0) * trueScale;
                                height = *(float*)(boundsStruct + 0xB4) * trueScale;
                            }
                        }
                    }
                }
                else
                {
                    if (!Game::IsUnitInLosTo(player, current))
                    {
                        current = *reinterpret_cast<uint32_t*>(current + 0x3C);
                        continue;
                    }

                    /*
                    if (!Game::IsUnitInteractable(current))
                    {
                        current = *reinterpret_cast<uint32_t*>(current + 0x3C);
                        continue;
                    }
                    */

                    uint32_t movement = *reinterpret_cast<uint32_t*>(current + 0xD8);
                    if (movement)
                    {
                        radius = *reinterpret_cast<float*>(movement + 0xC8) * trueScale;
                        height = *reinterpret_cast<float*>(movement + 0xCC) * trueScale;
                    }

                    uintptr_t model = *(uintptr_t*)(current + 0x970);
                    if (model)
                    {
                        radius = (*(float*)(model + 0x54) * *(float*)(model + 0x220)) * trueScale;
                        height = ((*(float*)(model + 0x58) * *(float*)(model + 0x220)) * *(float*)(model + 0xD0)) * trueScale;
                    }

                    if (Game::IsUnitDead(current))
                    {
                        height /= 4;
                    }
                }

                // Bounding box
                C3Vector topCorners[4] = {
                    { worldPos.x - radius, worldPos.y - radius, worldPos.z + height },
                    { worldPos.x + radius, worldPos.y - radius, worldPos.z + height },
                    { worldPos.x + radius, worldPos.y + radius, worldPos.z + height },
                    { worldPos.x - radius, worldPos.y + radius, worldPos.z + height }
                };
                C3Vector bottomCorners[4] = {
                    { worldPos.x - radius, worldPos.y - radius, worldPos.z },
                    { worldPos.x + radius, worldPos.y - radius, worldPos.z },
                    { worldPos.x + radius, worldPos.y + radius, worldPos.z },
                    { worldPos.x - radius, worldPos.y + radius, worldPos.z }
                };

                // Project corners
                C2Vector projected[8];
                bool allVisible = true;
                for (int i = 0; i < 4; ++i)
                {
                    if (!Game::WorldToScreen(topCorners[i].x, topCorners[i].y, topCorners[i].z, projected[i]) ||
                        !Game::WorldToScreen(bottomCorners[i].x, bottomCorners[i].y, bottomCorners[i].z, projected[i + 4]))
                    {
                        allVisible = false;
                        break;
                    }
                }

                if (!allVisible)
                {
                    current = *reinterpret_cast<uint32_t*>(current + 0x3C);
                    continue;
                }

                float minX = projected[0].x, maxX = projected[0].x;
                float minY = projected[0].y, maxY = projected[0].y;
                for (int i = 1; i < 8; ++i)
                {
                    minX = std::min(minX, projected[i].x);
                    maxX = std::max(maxX, projected[i].x);
                    minY = std::min(minY, projected[i].y);
                    maxY = std::max(maxY, projected[i].y);
                }

                if (centerX >= minX && centerX <= maxX && centerY >= minY && centerY <= maxY)
                {
                    bestTarget = current;
                    break;
                }
            }

            current = *reinterpret_cast<uint32_t*>(current + 0x3C);
        }

        return bestTarget;
    }

    bool TargetNearestEnemy(float distanceLimit) {
        std::vector<struct mob_entity> mobs;

        uint32_t objectsBase = *reinterpret_cast<uint32_t*>(Offsets::VISIBLE_OBJECTS);
        uint32_t objects = *reinterpret_cast<uint32_t*>(objectsBase + 0x2ED0);

        uint32_t i = *reinterpret_cast<uint32_t*>(objects + 0xAC);

        uint64_t playerGUID = *reinterpret_cast<uint64_t*>(objects + 0xC0);
        uint32_t player = Game::GetObjectPointer(playerGUID);

        C3Vector pPos = Game::GetUnitPosition(player);
        C3Vector oPos;

        while (i != 0 && (i & 1) == 0) {
            uint64_t guid = *reinterpret_cast<uint64_t*>(i + 0x30);
            uint32_t pointer = Game::GetObjectPointer(guid);
            uint32_t type = *reinterpret_cast<uint32_t*>(pointer + 0x14);


            // As of https://github.com/allfoxwy/UnitXP_SP3/issues/5
            // I suspect some in-game object on battleground can not get position as Unit, so I add this line and it does stop crashing
            if (type != ObjectType::PLAYER && type != ObjectType::UNIT) {
                i = *reinterpret_cast<uint32_t*>(*reinterpret_cast<int32_t*>(objects + 0xa4) + i + 4);
                continue;
            }

            oPos = Game::GetUnitPosition(i);

            float distance = distance3D(oPos, pPos);
            bool targetInCombat = Game::IsUnitInCombat(i);
            bool inViewFrustum = Game::inViewingFrustum(playerGUID, oPos, targetingRangeCone);
            bool inLos = Game::IsUnitInLosTo(player, i);

            if (((type == ObjectType::UNIT && !Game::IsUnitControlledByPlayer(i)) || type == ObjectType::PLAYER)
                && distance <= distanceLimit
                && Game::UnitCanBeAttacked(player, i)
                && !Game::IsUnitDead(i)
                && (targetInCombat == true || Game::UnitCreatureType(i) != CREATURE_TYPE_CRITTER)
                && Game::inViewingFrustum(playerGUID, oPos, targetingRangeCone)
                && Game::IsUnitInLosTo(player, i)) {

                bool selfInCombat = Game::IsUnitInCombat(player);

                if (type == ObjectType::UNIT && selfInCombat) {
                    if (targetInCombat) {
                        struct mob_entity new_mob = {};
                        new_mob.GUID = guid;
                        new_mob.distance = distance;
                        mobs.push_back(new_mob);
                    }
                }
                else {
                    struct mob_entity new_mob = {};
                    new_mob.GUID = guid;
                    new_mob.distance = distance;
                    mobs.push_back(new_mob);
                }

            }
            i = *reinterpret_cast<uint32_t*>(*reinterpret_cast<int32_t*>(objects + 0xa4) + i + 4);
        }

        if (mobs.size() > 0) {
            auto compareFunction = [](struct mob_entity& a, struct mob_entity& b) {
                return a.distance < b.distance;
            };
            std::sort(mobs.begin(), mobs.end(), compareFunction);

            Game::SetTarget(mobs.front().GUID);
            return true;
        }

        return false;
    }

    bool TargetNearestEnemyFacing(float distanceLimit, uint64_t* outGuid, float targetingCone) {
        std::vector<struct mob_entity> mobs;

        uint32_t objectsBase = *reinterpret_cast<uint32_t*>(Offsets::VISIBLE_OBJECTS);
        uint32_t objects = *reinterpret_cast<uint32_t*>(objectsBase + 0x2ED0);

        uint32_t currentObject = *reinterpret_cast<uint32_t*>(objects + 0xAC);

        uint64_t playerGUID = *reinterpret_cast<uint64_t*>(objects + 0xC0);
        uint32_t player = Game::GetObjectPointer(playerGUID);

        C3Vector pPos = Game::GetUnitPosition(player);
        C3Vector oPos;

        while (currentObject != 0 && (currentObject & 1) == 0) {
            uint64_t guid = *reinterpret_cast<uint64_t*>(currentObject + 0x30);
            uint32_t pointer = Game::GetObjectPointer(guid);
            uint32_t type = *reinterpret_cast<uint32_t*>(pointer + 0x14);

            if (type != ObjectType::PLAYER && type != ObjectType::UNIT) {
                currentObject = *reinterpret_cast<uint32_t*>(currentObject + 0x3C); // Skip
                continue;
            }

            oPos = Game::GetUnitPosition(currentObject);

            float distance = distance3D(oPos, pPos);

            if (distance <= distanceLimit &&
                Game::UnitCanBeAttacked(player, currentObject) &&
                !Game::IsUnitDead(currentObject) &&
                Game::inViewingFrustum(playerGUID, oPos, targetingCone) &&
                Game::IsFacing(player, currentObject, targetingCone)) {

                struct mob_entity new_mob = { guid, distance };
                mobs.push_back(new_mob);
            }

            currentObject = *reinterpret_cast<uint32_t*>(currentObject + 0x3C); // Move to next
        }

        if (!mobs.empty()) {
            std::sort(mobs.begin(), mobs.end(), [](const mob_entity& a, const mob_entity& b) {
                return a.distance < b.distance;
            });

            *outGuid = mobs.front().GUID;

            return true;
        }

        return false;
    }


    bool TargetWorldBoss(float distanceLimit) {
        std::vector<struct mob_entity> mobs;

        uint32_t objectsBase = *reinterpret_cast<uint32_t*>(Offsets::VISIBLE_OBJECTS);
        uint32_t objects = *reinterpret_cast<uint32_t*>(objectsBase + 0x2ED0);

        uint32_t i = *reinterpret_cast<uint32_t*>(objects + 0xAC);

        uint64_t playerGUID = *reinterpret_cast<uint64_t*>(objects + 0xC0);
        uint32_t player = Game::GetObjectPointer(playerGUID);

        C3Vector pPos = Game::GetUnitPosition(player);
        C3Vector oPos;

        while (i != 0 && (i & 1) == 0) {
            uint64_t guid = *reinterpret_cast<uint64_t*>(i + 0x30);
            uint32_t pointer = Game::GetObjectPointer(guid);
            uint32_t type = *reinterpret_cast<uint32_t*>(pointer + 0x14);

            // As of https://github.com/allfoxwy/UnitXP_SP3/issues/5
            // I suspect some in-game object on battleground can not get position as Unit, so I add this line and it does stop crashing
            if (type != ObjectType::PLAYER && type != ObjectType::UNIT) {
                i = *reinterpret_cast<uint32_t*>(*reinterpret_cast<int32_t*>(objects + 0xa4) + i + 4);
                continue;
            }

            oPos = Game::GetUnitPosition(i);

            float distance = distance3D(oPos, pPos);
            bool targetInCombat = Game::IsUnitInCombat(i);

            if (((type == ObjectType::UNIT && !Game::IsUnitControlledByPlayer(i)) || type == ObjectType::PLAYER)
                && distance <= distanceLimit
                && Game::UnitCanBeAttacked(player, i)
                && !Game::IsUnitDead(i)
                && Game::UnitCreatureRank(i) == CREATURERANK_WORLDBOSS
                && (targetInCombat == true || Game::UnitCreatureType(i) != CREATURE_TYPE_CRITTER)
                && Game::inViewingFrustum(playerGUID, oPos, targetingRangeCone)) {

                bool selfInCombat = Game::IsUnitInCombat(player);

                if (type == ObjectType::UNIT && selfInCombat) {
                    if (targetInCombat) {
                        struct mob_entity new_mob = {};
                        new_mob.GUID = guid;
                        new_mob.distance = distance;
                        mobs.push_back(new_mob);
                    }
                }
                else {
                    struct mob_entity new_mob = {};
                    new_mob.GUID = guid;
                    new_mob.distance = distance;
                    mobs.push_back(new_mob);
                }

            }
            i = *reinterpret_cast<uint32_t*>(*reinterpret_cast<int32_t*>(objects + 0xa4) + i + 4);
        }

        if (mobs.size() > 0) {
            auto compareFunction = [](struct mob_entity& a, struct mob_entity& b) {
                return a.distance < b.distance;
            };
            std::sort(mobs.begin(), mobs.end(), compareFunction);

            // TODO: Maybe we could find last target without lagging
            //static uint64_t lastTarget = 0;
            uint64_t lastTarget = Game::UnitTargetGuid(player);

            bool lastTargetIsBoss = false;
            for (auto const& m : mobs) {
                if (m.GUID == lastTarget) {
                    lastTargetIsBoss = true;
                    break;
                }
            }

            if (lastTarget == 0 || lastTargetIsBoss == false) {
                Game::SetTarget(mobs.front().GUID);
                return true;
            }
            else {
                auto nextGUID = SelectNext(lastTarget, mobs);
                Game::SetTarget(nextGUID);
                return true;
            }
        }

        return false;
    }

    // Return the mark's order in list
    static unsigned int markInList(const int mark, const std::vector<int>& list) {
        for (unsigned int i = 0; i < list.size(); ++i) {
            if (list[i] == mark) {
                return i;
            }
        }
        return UINT_MAX;
    }

    uint64_t SelectNext(uint64_t current, std::vector<struct mob_entity>& list) {
        if (list.size() == 0) {
            return 0;
        }

        auto compareFunction = [](const struct mob_entity& a, const struct mob_entity& b) {
            return a.GUID < b.GUID;
        };
        std::sort(list.begin(), list.end(), compareFunction);

        for (auto i = list.begin(); i < list.end(); ++i) {
            if ((*i).GUID == current) {
                if (i < list.end() - 1) {
                    return (*(i + 1)).GUID;
                }
                else {
                    break;
                }
            }
        }

        return list.front().GUID;
    }

    uint64_t SelectPrevious(uint64_t current, std::vector<struct mob_entity>& list) {
        if (list.size() == 0) {
            return 0;
        }

        auto compareFunction = [](const struct mob_entity& a, const struct mob_entity& b) {
            return a.GUID > b.GUID;
        };
        std::sort(list.begin(), list.end(), compareFunction);

        for (auto i = list.begin(); i < list.end(); ++i) {
            if ((*i).GUID == current) {
                if (i < list.end() - 1) {
                    return (*(i + 1)).GUID;
                }
                else {
                    break;
                }
            }
        }

        return list.front().GUID;
    }

    uint64_t SelectNextMark(const uint64_t current, std::vector<struct mob_entity>& list, const std::vector<int>& priority) {
        if (list.size() == 0) {
            return 0;
        }

        auto compareFunction = [&priority](const struct mob_entity& a, const struct mob_entity& b) {
            return markInList(a.targetMark, priority) < markInList(b.targetMark, priority);
        };
        std::sort(list.begin(), list.end(), compareFunction);

        for (auto i = list.begin(); i < list.end(); ++i) {
            if ((*i).GUID == current) {
                if (i < list.end() - 1) {
                    return (*(i + 1)).GUID;
                }
                else {
                    break;
                }
            }
        }

        return list.front().GUID;
    }

    uint64_t SelectPreviousMark(const uint64_t current, std::vector<struct mob_entity>& list, const std::vector<int>& priority) {
        if (list.size() == 0) {
            return 0;
        }

        auto compareFunction = [&priority](const struct mob_entity& a, const struct mob_entity& b) {
            return markInList(a.targetMark, priority) > markInList(b.targetMark, priority);
        };
        sort(list.begin(), list.end(), compareFunction);

        for (auto i = list.begin(); i < list.end(); ++i) {
            if ((*i).GUID == current) {
                if (i < list.end() - 1) {
                    return (*(i + 1)).GUID;
                }
                else {
                    break;
                }
            }
        }

        return list.front().GUID;
    }

    bool TargetEnemyInCycle(MOB_SELECTFUNCTION selectFunction) {
        if (!selectFunction) {
            return false;
        }

        std::vector<struct mob_entity> list;

        uint32_t objectsBase = *reinterpret_cast<uint32_t*>(Offsets::VISIBLE_OBJECTS);
        uint32_t objects = *reinterpret_cast<uint32_t*>(objectsBase + 0x2ED0);

        uint32_t i = *reinterpret_cast<uint32_t*>(objects + 0xAC);

        uint64_t playerGUID = *reinterpret_cast<uint64_t*>(objects + 0xC0);
        uint32_t player = Game::GetObjectPointer(playerGUID);

        C3Vector pPos = Game::GetUnitPosition(player);
        C3Vector oPos;

        // TODO: Maybe we could find last target without lagging
        //static uint64_t lastTarget = 0;
        uint64_t lastTarget = Game::UnitTargetGuid(player);

        if (lastTarget == 0) {
            return TargetNearestEnemy(41.0f);
        }

        while (i != 0 && (i & 1) == 0) {
            uint64_t guid = *reinterpret_cast<uint64_t*>(i + 0x30);
            uint32_t pointer = Game::GetObjectPointer(guid);
            uint32_t type = *reinterpret_cast<uint32_t*>(pointer + 0x14);


            // As of https://github.com/allfoxwy/UnitXP_SP3/issues/5
            // I suspect some in-game object on battleground can not get position as Unit, so I add this line and it does stop crashing
            if (type != ObjectType::PLAYER && type != ObjectType::UNIT) {
                i = *reinterpret_cast<uint32_t*>(*reinterpret_cast<int32_t*>(objects + 0xa4) + i + 4);
                continue;
            }

            oPos = Game::GetUnitPosition(i);

            float distance = distance3D(oPos, pPos);
            bool targetInCombat = Game::IsUnitInCombat(i);

            if (((type == ObjectType::UNIT && !Game::IsUnitControlledByPlayer(i)) || type == ObjectType::PLAYER)
                && distance <= 41.0f
                && Game::UnitCanBeAttacked(player, i)
                && !Game::IsUnitDead(i)
                && (targetInCombat == true || Game::UnitCreatureType(i) != CREATURE_TYPE_CRITTER)
                && Game::inViewingFrustum(playerGUID, oPos, targetingRangeCone)
                && Game::IsUnitInLosTo(player, i)) {

                bool selfInCombat = Game::IsUnitInCombat(player);

                if (type == ObjectType::UNIT && selfInCombat) {
                    if (targetInCombat) {
                        struct mob_entity new_mob = {};
                        new_mob.GUID = guid;
                        new_mob.distance = distance;
                        list.push_back(new_mob);
                    }
                }
                else {
                    struct mob_entity new_mob = {};
                    new_mob.GUID = guid;
                    new_mob.distance = distance;
                    list.push_back(new_mob);
                }

            }
            i = *reinterpret_cast<uint32_t*>(*reinterpret_cast<int32_t*>(objects + 0xa4) + i + 4);
        }

        if (list.size() > 0) {
            uint64_t choice = selectFunction(lastTarget, list);
            Game::SetTarget(choice);
            return true;
        }

        return false;
    }

    bool TargetMarkedEnemyInCycle(MOB_SELECTFUNCTION_WITH_MARK selectFunction, std::string priority) {
        if (!selectFunction) {
            return false;
        }

        std::vector<struct mob_entity> list;
        std::vector<int> priorityList;

        for (unsigned int i = 0; i < std::min(8u, priority.length()); ++i) {
            if (priority[i] >= '1' && priority[i] <= '8') {
                priorityList.push_back(priority[i] - 48);
            }
        }

        if (priorityList.size() == 0) {
            for (int i = 8; i >= 1; --i) {
                priorityList.push_back(i);
            }
        }

        uint32_t objectsBase = *reinterpret_cast<uint32_t*>(Offsets::VISIBLE_OBJECTS);
        uint32_t objects = *reinterpret_cast<uint32_t*>(objectsBase + 0x2ED0);

        uint32_t i = *reinterpret_cast<uint32_t*>(objects + 0xAC);

        uint64_t playerGUID = *reinterpret_cast<uint64_t*>(objects + 0xC0);
        uint32_t player = Game::GetObjectPointer(playerGUID);

        C3Vector pPos = Game::GetUnitPosition(player);
        C3Vector oPos;

        // TODO: Maybe we could find last target without lagging
        //static uint64_t lastTarget = 0;
        uint64_t lastTarget = Game::UnitTargetGuid(player);

        while (i != 0 && (i & 1) == 0) {
            uint64_t guid = *reinterpret_cast<uint64_t*>(i + 0x30);
            uint32_t pointer = Game::GetObjectPointer(guid);
            uint32_t type = *reinterpret_cast<uint32_t*>(pointer + 0x14);


            // As of https://github.com/allfoxwy/UnitXP_SP3/issues/5
            // I suspect some in-game object on battleground can not get position as Unit, so I add this line and it does stop crashing
            if (type != ObjectType::PLAYER && type != ObjectType::UNIT) {
                i = *reinterpret_cast<uint32_t*>(*reinterpret_cast<int32_t*>(objects + 0xa4) + i + 4);
                continue;
            }

            oPos = Game::GetUnitPosition(i);

            float distance = distance3D(oPos, pPos);
            bool targetInCombat = Game::IsUnitInCombat(i);
            int mark = Game::UnitRaidMarker(i);

            if (((type == ObjectType::UNIT && !Game::IsUnitControlledByPlayer(i)) || type == ObjectType::PLAYER)
                && mark > 0
                && Game::UnitCanBeAttacked(player, i)
                && !Game::IsUnitDead(i)
                && (targetInCombat == true || Game::UnitCreatureType(i) != CREATURE_TYPE_CRITTER)
                && Game::inViewingFrustum(playerGUID, oPos, targetingRangeCone)) {

                bool selfInCombat = Game::IsUnitInCombat(player);

                if (type == ObjectType::UNIT && selfInCombat) {
                    if (targetInCombat) {
                        struct mob_entity new_mob = {};
                        new_mob.GUID = guid;
                        new_mob.distance = distance;
                        new_mob.targetMark = mark;
                        if (markInList(new_mob.targetMark, priorityList) < priorityList.size()) {
                            list.push_back(new_mob);
                        }
                    }
                }
                else {
                    struct mob_entity new_mob = {};
                    new_mob.GUID = guid;
                    new_mob.distance = distance;
                    new_mob.targetMark = mark;
                    if (markInList(new_mob.targetMark, priorityList) < priorityList.size()) {
                        list.push_back(new_mob);
                    }
                }

            }
            i = *reinterpret_cast<uint32_t*>(*reinterpret_cast<int32_t*>(objects + 0xa4) + i + 4);
        }

        if (list.size() > 0) {
            uint64_t choice = selectFunction(lastTarget, list, priorityList);
            Game::SetTarget(choice);
            return true;
        }

        return false;
    }

    bool TargetEnemyConsideringDistance(MOB_SELECTFUNCTION selectFunction) {
        if (!selectFunction) {
            return false;
        }

        std::vector<struct mob_entity> meleeRange;
        std::vector<struct mob_entity> chargeRange;
        std::vector<struct mob_entity> farRange;

        uint32_t objectsBase = *reinterpret_cast<uint32_t*>(Offsets::VISIBLE_OBJECTS);
        uint32_t objects = *reinterpret_cast<uint32_t*>(objectsBase + 0x2ED0);

        uint32_t i = *reinterpret_cast<uint32_t*>(objects + 0xAC);

        uint64_t playerGUID = *reinterpret_cast<uint64_t*>(objects + 0xC0);
        uint32_t player = Game::GetObjectPointer(playerGUID);

        C3Vector pPos = Game::GetUnitPosition(player);
        C3Vector oPos;

        // TODO: Maybe we could find last target without lagging
        //static uint64_t lastTarget = 0;
        uint64_t lastTarget = Game::UnitTargetGuid(player);

        if (lastTarget == 0) {
            return TargetNearestEnemy(41.0f);
        }

        while (i != 0 && (i & 1) == 0) {
            uint64_t guid = *reinterpret_cast<uint64_t*>(i + 0x30);
            uint32_t pointer = Game::GetObjectPointer(guid);
            uint32_t type = *reinterpret_cast<uint32_t*>(pointer + 0x14);


            // As of https://github.com/allfoxwy/UnitXP_SP3/issues/5
            // I suspect some in-game object on battleground can not get position as Unit, so I add this line and it does stop crashing
            if (type != ObjectType::PLAYER && type != ObjectType::UNIT) {
                i = *reinterpret_cast<uint32_t*>(*reinterpret_cast<int32_t*>(objects + 0xa4) + i + 4);
                continue;
            }

            oPos = Game::GetUnitPosition(i);

            float distance = distance3D(oPos, pPos);
            bool targetInCombat = Game::IsUnitInCombat(i);

            if (((type == ObjectType::UNIT && !Game::IsUnitControlledByPlayer(i)) || type == ObjectType::PLAYER)
                && distance <= 41.0f
                && Game::UnitCanBeAttacked(player, i)
                && !Game::IsUnitDead(i)
                && (targetInCombat == true || Game::UnitCreatureType(i) != CREATURE_TYPE_CRITTER)
                && Game::inViewingFrustum(playerGUID, oPos, targetingRangeCone)
                && Game::IsUnitInLosTo(player, i)) {

                bool selfInCombat = Game::IsUnitInCombat(player);

                if (type == ObjectType::UNIT && selfInCombat) {
                    if (targetInCombat) {
                        struct mob_entity new_mob = {};
                        new_mob.GUID = guid;
                        new_mob.distance = distance;

                        if (new_mob.distance <= 8.0f) {
                            meleeRange.push_back(new_mob);
                        }
                        else if (new_mob.distance <= 25.0f) {
                            chargeRange.push_back(new_mob);
                        }
                        else if (new_mob.distance < 41.0f) {
                            farRange.push_back(new_mob);
                        }
                    }
                }
                else {
                    struct mob_entity new_mob = {};
                    new_mob.GUID = guid;
                    new_mob.distance = distance;

                    if (new_mob.distance < 8.0f) {
                        meleeRange.push_back(new_mob);
                    }
                    else if (new_mob.distance < 25.0f) {
                        chargeRange.push_back(new_mob);
                    }
                    else if (new_mob.distance < 41.0f) {
                        farRange.push_back(new_mob);
                    }
                }

            }
            i = *reinterpret_cast<uint32_t*>(*reinterpret_cast<int32_t*>(objects + 0xa4) + i + 4);
        }

        if (meleeRange.size() > 0) {
            uint64_t choice = selectFunction(lastTarget, meleeRange);
            lastTarget = choice;
            Game::SetTarget(choice);
            return true;
        }

        if (chargeRange.size() > 0) {
            const int limit = 3;
            if (chargeRange.size() > limit) {
                auto compareFunction = [](struct mob_entity& a, struct mob_entity& b) {
                    return a.distance < b.distance;
                };
                sort(chargeRange.begin(), chargeRange.end(), compareFunction);
                chargeRange.erase(chargeRange.begin() + limit, chargeRange.end());
            }

            uint64_t choice = selectFunction(lastTarget, chargeRange);

            lastTarget = choice;
            Game::SetTarget(choice);
            return true;
        }
        if (farRange.size() > 0) {
            const int limit = 5;
            if (farRange.size() > limit) {
                auto compareFunction = [](struct mob_entity& a, struct mob_entity& b) {
                    return a.distance < b.distance;
                };
                sort(farRange.begin(), farRange.end(), compareFunction);
                farRange.erase(farRange.begin() + limit, farRange.end());
            }


            uint64_t choice = selectFunction(lastTarget, farRange);

            lastTarget = choice;
            Game::SetTarget(choice);
            return true;
        }

        return false;
    }
}