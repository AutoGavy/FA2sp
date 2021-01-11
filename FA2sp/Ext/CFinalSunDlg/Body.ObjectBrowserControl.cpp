#include "Body.h"

#include "../../Helpers/Translations.h"
#include "../../Helpers/STDHelpers.h"
#include "../../Helpers/MultimapHelper.h"

#include "../../Logger.h"

#include <GlobalVars.h>

std::unordered_map<int, HTREEITEM> ObjectBrowserControlExt::ExtNodes;
std::unordered_set<std::string> ObjectBrowserControlExt::IgnoreSet;
std::unordered_set<std::string> ObjectBrowserControlExt::ExtSets[Set_Count];
std::unordered_map<std::string, int> ObjectBrowserControlExt::KnownItem;

HTREEITEM ObjectBrowserControlExt::InsertString(const char* pString, DWORD dwItemData,
    HTREEITEM hParent, HTREEITEM hInsertAfter)
{
    return this->InsertItem(5, pString, 0, 0, 0, 0, dwItemData, hParent, hInsertAfter);
}

HTREEITEM ObjectBrowserControlExt::InsertTranslatedString(const char* pOriginString, DWORD dwItemData,
    HTREEITEM hParent, HTREEITEM hInsertAfter)
{
    CString buffer;
    bool result = Translations::GetTranslationItem(pOriginString, buffer);
    return InsertString(result ? buffer : pOriginString, dwItemData, hParent, hInsertAfter);
}

// ObjectBrowserControlExt::Redraw

void ObjectBrowserControlExt::Redraw()
{
    Redraw_Initialize();
    Redraw_MainList();
    Redraw_Ground();
    Redraw_Owner();
    Redraw_Infantry();
    Redraw_Vehicle();
    Redraw_Aircraft();
    Redraw_Building();
    Redraw_Terrain();
    Redraw_Smudge();
    Redraw_Overlay();
    Redraw_Waypoint();
    Redraw_Celltag();
    Redraw_Basenode();
    Redraw_Tunnel();
    Redraw_PlayerLocation();
}

void ObjectBrowserControlExt::Redraw_Initialize()
{
    ExtNodes.clear();
    KnownItem.clear();
    IgnoreSet.clear();
    this->DeleteAllItems();

    auto& rules = GlobalVars::INIFiles::Rules();
    auto& fadata = GlobalVars::INIFiles::FAData();
    auto& doc = GlobalVars::INIFiles::CurrentDocument();

    auto loadSet = [&rules, &doc](const char* pTypeName, int nType)
    {
        ExtSets[nType].clear();
        if (rules.SectionExists(pTypeName))
        {
            auto& section = rules.GetSection(pTypeName).EntitiesDictionary;
            for (auto& itr : section)
                ExtSets[nType].insert((std::string)itr.second);
        }
        if (doc.SectionExists(pTypeName))
        {
            auto& section = doc.GetSection(pTypeName).EntitiesDictionary;
            for (auto& itr : section)
                ExtSets[nType].insert((std::string)itr.second);
        }
    };

    loadSet("BuildingTypes", Set_Building);
    loadSet("InfantryTypes", Set_Infantry);
    loadSet("VehicleTypes", Set_Vehicle);
    loadSet("AircraftTypes", Set_Aircraft);

    if (fadata.SectionExists("ForceSides"))
    {
        auto& knownSection = fadata.GetSection("ForceSides").EntitiesDictionary;
        for (auto& item : knownSection)
        {
            int sideIndex = STDHelpers::ParseToInt(item.second, -1);
            if (sideIndex >= rules.GetKeyCount("Sides"))
                continue;
            if (sideIndex < -1)
                sideIndex = -1;
            KnownItem[(std::string)item.first] = sideIndex;
        }
    }

    if (fadata.SectionExists("IgnoreRA2"))
    {
        auto& ignores = fadata.GetSection("IgnoreRA2").EntitiesDictionary;
        for (auto& item : ignores)
            IgnoreSet.insert((std::string)item.second);
    }
}

void ObjectBrowserControlExt::Redraw_MainList()
{
    ExtNodes[Root_Nothing] = this->InsertTranslatedString("NothingObList", -2);
    ExtNodes[Root_Ground] = this->InsertTranslatedString("GroundObList", 13);
    ExtNodes[Root_Owner] = this->InsertTranslatedString("ChangeOwnerObList");
    ExtNodes[Root_Infantry] = this->InsertTranslatedString("InfantryObList", 0);
    ExtNodes[Root_Vehicle] = this->InsertTranslatedString("VehiclesObList", 1);
    ExtNodes[Root_Aircraft] = this->InsertTranslatedString("AircraftObList", 2);
    ExtNodes[Root_Building] = this->InsertTranslatedString("StructuresObList", 3);
    ExtNodes[Root_Terrain] = this->InsertTranslatedString("TerrainObList", 4);
    ExtNodes[Root_Smudge] = this->InsertTranslatedString("SmudgesObList", 10);
    ExtNodes[Root_Overlay] = this->InsertTranslatedString("OverlayObList", 5);
    ExtNodes[Root_Waypoint] = this->InsertTranslatedString("WaypointsObList", 6);
    ExtNodes[Root_Celltag] = this->InsertTranslatedString("CelltagsObList", 7);
    ExtNodes[Root_Basenode] = this->InsertTranslatedString("BaseNodesObList", 8);
    ExtNodes[Root_Tunnel] = this->InsertTranslatedString("TunnelObList", 9);
    ExtNodes[Root_PlayerLocation] = this->InsertTranslatedString("StartpointsObList", 12);
    ExtNodes[Root_Delete] = this->InsertTranslatedString("DelObjObList", 10);
}

void ObjectBrowserControlExt::Redraw_Ground()
{
    HTREEITEM& hGround = ExtNodes[Root_Ground];
    if (hGround == NULL)    return;

    auto& doc = GlobalVars::INIFiles::CurrentDocument();
    CString theater = doc.GetString("Map", "Theater");
    if (theater == "NEWURBAN")
        theater = "UBN";

    CString suffix;
    if (theater != "")
        suffix = theater.Mid(0, 3);

    this->InsertTranslatedString("GroundClearObList" + suffix, 61, hGround);
    this->InsertTranslatedString("GroundSandObList"  + suffix, 62, hGround);
    this->InsertTranslatedString("GroundRoughObList" + suffix, 63, hGround);
    this->InsertTranslatedString("GroundGreenObList" + suffix, 65, hGround);
    this->InsertTranslatedString("GroundPaveObList"  + suffix, 66, hGround);
    this->InsertTranslatedString("GroundWaterObList", 64, hGround);

    if(theater == "UBN")
        this->InsertTranslatedString("GroundPave2ObListUBN", 67, hGround);
}

void ObjectBrowserControlExt::Redraw_Owner()
{
    HTREEITEM& hOwner = ExtNodes[Root_Owner];
    if (hOwner == NULL)    return;

    auto& doc = GlobalVars::INIFiles::CurrentDocument();
    bool bMultiplayer = doc.GetBool("Basic", "MultiplayerOnly");

    if (!doc.SectionExists("Houses"))
        return;
    auto& section = doc.GetSection("Houses").EntitiesDictionary;
    for (auto& house : section)
    {
        int index = STDHelpers::ParseToInt(house.first, -1);
        if (index == -1)
            continue;
        if (bMultiplayer)
            if (house.second != "Neutral" && house.second != "Special")
                continue;
        /*CString uiname = house.second.Mid(0, house.second.Find(' '));
        CString queryname = CSFQuery::GetUIName(uiname);
        this->InsertString(
            (uiname == queryname) && (queryname != "MISSING") ? house.second : queryname,
            Const_House + index,
            hOwner
        );*/
        this->InsertString(house.second, Const_House + index, hOwner);
    }
}

void ObjectBrowserControlExt::Redraw_Infantry()
{
    HTREEITEM& hInfantry = ExtNodes[Root_Infantry];
    if (hInfantry == NULL)   return;

    std::unordered_map<int, HTREEITEM> subNodes;

    auto& fadata = GlobalVars::INIFiles::FAData();

    int i = 0;
    if (!fadata.SectionExists("Sides"))
    {
        subNodes[i++] = this->InsertString("Allied", -1, hInfantry);
        subNodes[i++] = this->InsertString("Soviet", -1, hInfantry);
        subNodes[i++] = this->InsertString("Yuri", -1, hInfantry);
    }
    else
    {
        auto& sides = fadata.GetSection("Sides").EntitiesDictionary;
        for (auto& itr : sides)
            subNodes[i++] = this->InsertString(itr.second, -1, hInfantry);
    }
    subNodes[-1] = this->InsertString("Others", -1, hInfantry);

    MultimapHelper mmh;
    mmh.AddINI(&GlobalVars::INIFiles::Rules());
    mmh.AddINI(&GlobalVars::INIFiles::CurrentDocument());

    auto& infantries = mmh.GetSection("InfantryTypes");
    for (auto& inf : infantries)
    {
        if (IgnoreSet.find((std::string)inf.second) != IgnoreSet.end())
            continue;
        int index = STDHelpers::ParseToInt(inf.first, -1);
        if (index == -1)   continue;
        int side = GuessSide(inf.second, Set_Infantry);
        if (subNodes.find(side) == subNodes.end())
            side = -1;
        this->InsertString(
            CSFQuery::GetUIName(inf.second),
            Const_Infantry + index,
            subNodes[side]
        );
    }
}

void ObjectBrowserControlExt::Redraw_Vehicle()
{
    HTREEITEM& hVehicle = ExtNodes[Root_Vehicle];
    if (hVehicle == NULL)   return;

    std::unordered_map<int, HTREEITEM> subNodes;

    auto& fadata = GlobalVars::INIFiles::FAData();

    int i = 0;
    if (!fadata.SectionExists("Sides"))
    {
        subNodes[i++] = this->InsertString("Allied", -1, hVehicle);
        subNodes[i++] = this->InsertString("Soviet", -1, hVehicle);
        subNodes[i++] = this->InsertString("Yuri", -1, hVehicle);
    }
    else
    {
        auto& sides = fadata.GetSection("Sides").EntitiesDictionary;
        for (auto& itr : sides)
            subNodes[i++] = this->InsertString(itr.second, -1, hVehicle);
    }
    subNodes[-1] = this->InsertString("Others", -1, hVehicle);

    MultimapHelper mmh;
    mmh.AddINI(&GlobalVars::INIFiles::Rules());
    mmh.AddINI(&GlobalVars::INIFiles::CurrentDocument());

    auto& vehicles = mmh.GetSection("VehicleTypes");
    for (auto& veh : vehicles)
    {
        if (IgnoreSet.find((std::string)veh.second) != IgnoreSet.end())
            continue;
        int index = STDHelpers::ParseToInt(veh.first, -1);
        if (index == -1)   continue;
        int side = GuessSide(veh.second, Set_Vehicle);
        if (subNodes.find(side) == subNodes.end())
            side = -1;
        this->InsertString(
            CSFQuery::GetUIName(veh.second),
            Const_Vehicle + index,
            subNodes[side]
        );
    }
}

void ObjectBrowserControlExt::Redraw_Aircraft()
{
    HTREEITEM& hAircraft = ExtNodes[Root_Aircraft];
    if (hAircraft == NULL)   return;

    std::unordered_map<int, HTREEITEM> subNodes;

    auto& rules = GlobalVars::INIFiles::Rules();
    auto& fadata = GlobalVars::INIFiles::FAData();

    int i = 0;
    if (!fadata.SectionExists("Sides"))
    {
        subNodes[i++] = this->InsertString("Allied", -1, hAircraft);
        subNodes[i++] = this->InsertString("Soviet", -1, hAircraft);
        subNodes[i++] = this->InsertString("Yuri", -1, hAircraft);
    }
    else
    {
        auto& sides = fadata.GetSection("Sides").EntitiesDictionary;
        for (auto& itr : sides)
            subNodes[i++] = this->InsertString(itr.second, -1, hAircraft);
    }
    subNodes[-1] = this->InsertString("Others", -1, hAircraft);

    MultimapHelper mmh;
    mmh.AddINI(&GlobalVars::INIFiles::Rules());
    mmh.AddINI(&GlobalVars::INIFiles::CurrentDocument());

    auto& aircrafts = mmh.GetSection("AircraftTypes");
    for (auto& air : aircrafts)
    {
        if (IgnoreSet.find((std::string)air.second) != IgnoreSet.end())
            continue;
        int index = STDHelpers::ParseToInt(air.first, -1);
        if (index == -1)   continue;
        int side = GuessSide(air.second, Set_Aircraft);
        if (subNodes.find(side) == subNodes.end())
            side = -1;
        this->InsertString(
            CSFQuery::GetUIName(air.second),
            Const_Aircraft + index,
            subNodes[side]
        );
    }
}

void ObjectBrowserControlExt::Redraw_Building()
{
    HTREEITEM& hBuilding = ExtNodes[Root_Building];
    if (hBuilding == NULL)   return;

    std::unordered_map<int, HTREEITEM> subNodes;

    auto& rules = GlobalVars::INIFiles::Rules();
    auto& fadata = GlobalVars::INIFiles::FAData();

    int i = 0;
    if (!fadata.SectionExists("Sides"))
    {
        subNodes[i++] = this->InsertString("Allied", -1, hBuilding);
        subNodes[i++] = this->InsertString("Soviet", -1, hBuilding);
        subNodes[i++] = this->InsertString("Yuri", -1, hBuilding);
    }
    else
    {
        auto& sides = fadata.GetSection("Sides").EntitiesDictionary;
        for (auto& itr : sides)
            subNodes[i++] = this->InsertString(itr.second, -1, hBuilding);
    }
    subNodes[-1] = this->InsertString("Others", -1, hBuilding);

    MultimapHelper mmh;
    mmh.AddINI(&GlobalVars::INIFiles::Rules());
    mmh.AddINI(&GlobalVars::INIFiles::CurrentDocument());

    auto& buildings = mmh.GetSection("BuildingTypes");
    for (auto& bud : buildings)
    {
        if (IgnoreSet.find((std::string)bud.second) != IgnoreSet.end())
            continue;
        int index = STDHelpers::ParseToInt(bud.first, -1);
        if (index == -1)   continue;
        int side = GuessSide(bud.second, Set_Building);
        if (subNodes.find(side) == subNodes.end())
            side = -1;
        this->InsertString(
            CSFQuery::GetUIName(bud.second),
            Const_Building + index,
            subNodes[side]
        );
    }
}

void ObjectBrowserControlExt::Redraw_Terrain()
{
    HTREEITEM& hTerrain = ExtNodes[Root_Terrain];
    if (hTerrain == NULL)   return;

    this->InsertTranslatedString("RndTreeObList", 50999, hTerrain);

    auto& rules = GlobalVars::INIFiles::Rules();

    if (!rules.SectionExists("TerrainTypes"))
        return;
    auto& terrains = rules.GetSection("TerrainTypes").EntitiesDictionary;
    for (auto& ter : terrains)
    {
        if (IgnoreSet.find((std::string)ter.second) != IgnoreSet.end())
            continue;
        //if (!rules.SectionExists(ter.second))
        //    continue;
        //if (!rules.KeyExists(ter.second, "SnowOccupationBits"))
        //    continue;
        int index = STDHelpers::ParseToInt(ter.first);
        if (index == -1)   continue;
        CString buffer;
        buffer = CSFQuery::GetUIName(ter.second);
        this->InsertTranslatedString(
            buffer,
            Const_Terrain + index,
            hTerrain
        );
    }

}

void ObjectBrowserControlExt::Redraw_Smudge()
{
    HTREEITEM& hSmudge = ExtNodes[Root_Smudge];
    if (hSmudge == NULL)   return;

    auto& rules = GlobalVars::INIFiles::Rules();

    if (!rules.SectionExists("SmudgeTypes"))
        return;
    auto& smudges = rules.GetSection("SmudgeTypes").EntitiesDictionary;
    for (auto& smu : smudges)
    {
        if (IgnoreSet.find((std::string)smu.second) != IgnoreSet.end())
            continue;
        //if (!rules.SectionExists(smu.second))
        //    continue;
        //if (!rules.GetKeyCount(smu.second))
        //    continue;
        int index = STDHelpers::ParseToInt(smu.first);
        if (index == -1)   continue;
        this->InsertString(
            smu.second,
            Const_Smudge + index,
            hSmudge
        );
    }

}

void ObjectBrowserControlExt::Redraw_Overlay()
{
    HTREEITEM& hOverlay = ExtNodes[Root_Overlay];
    if (hOverlay == NULL)   return;

    auto& rules = GlobalVars::INIFiles::Rules();

    HTREEITEM hTemp;
    hTemp = this->InsertTranslatedString("DelOvrlObList", -1, hOverlay);
    this->InsertTranslatedString("DelOvrl0ObList", 60100, hTemp);
    this->InsertTranslatedString("DelOvrl1ObList", 60101, hTemp);
    this->InsertTranslatedString("DelOvrl2ObList", 60102, hTemp);
    this->InsertTranslatedString("DelOvrl3ObList", 60103, hTemp);

    hTemp = this->InsertTranslatedString("GrTibObList", -1, hOverlay);
    this->InsertTranslatedString("DrawTibObList", 60210, hTemp);
    this->InsertTranslatedString("DrawTib2ObList", 60310, hTemp);

    hTemp = this->InsertTranslatedString("BridgesObList", -1, hOverlay);
    this->InsertTranslatedString("BigBridgeObList", 60500, hTemp);
    this->InsertTranslatedString("SmallBridgeObList", 60501, hTemp);
    this->InsertTranslatedString("BigTrackBridgeObList", 60502, hTemp);
    this->InsertTranslatedString("SmallConcreteBridgeObList", 60503, hTemp);

    // Walls
    HTREEITEM hWalls = this->InsertTranslatedString("OthObList", -1, hOverlay);

    hTemp = this->InsertTranslatedString("AllObList", -1, hOverlay);

    this->InsertTranslatedString("OvrlManuallyObList", 60001, hOverlay);
    this->InsertTranslatedString("OvrlDataManuallyObList", 60002, hOverlay);

    if (!rules.SectionExists("OverlayTypes"))
        return;
    auto& overlays = rules.GetSection("OverlayTypes").EntitiesDictionary;

    // a rough support for tracks
    this->InsertTranslatedString("Tracks", Const_Overlay + 39, hOverlay);
    
    for (auto& ovl : overlays)
    {
        if (IgnoreSet.find((std::string)ovl.second) != IgnoreSet.end())
            continue;
        //if (!rules.SectionExists(ovl.second))
        //    continue;
        int index = STDHelpers::ParseToInt(ovl.first, -1);
        if (index == -1)   continue;
        if (index >= 255 || index < 0) continue;
        if (rules.GetBool(ovl.second, "Wall"))
            this->InsertString(
                CSFQuery::GetUIName(ovl.second),
                Const_Overlay + index,
                hWalls
            );
        this->InsertString(
            ovl.second,
            Const_Overlay + index,
            hTemp
        );
    }

}

void ObjectBrowserControlExt::Redraw_Waypoint()
{
    HTREEITEM& hWaypoint = ExtNodes[Root_Waypoint];
    if (hWaypoint == NULL)   return;

    this->InsertTranslatedString("CreateWaypObList", 20, hWaypoint);
    this->InsertTranslatedString("DelWaypObList", 21, hWaypoint);
    this->InsertTranslatedString("CreateSpecWaypObList", 22, hWaypoint);
}

void ObjectBrowserControlExt::Redraw_Celltag()
{
    HTREEITEM& hCellTag = ExtNodes[Root_Celltag];
    if (hCellTag == NULL)   return;

    this->InsertTranslatedString("CreateCelltagObList", 36, hCellTag);
    this->InsertTranslatedString("DelCelltagObList", 37, hCellTag);
    this->InsertTranslatedString("CelltagPropObList", 38, hCellTag);
}

void ObjectBrowserControlExt::Redraw_Basenode()
{
    HTREEITEM& hBasenode = ExtNodes[Root_Basenode];
    if (hBasenode == NULL)   return;

    this->InsertTranslatedString("CreateNodeNoDelObList", 40, hBasenode);
    this->InsertTranslatedString("CreateNodeDelObList", 41, hBasenode);
    this->InsertTranslatedString("DelNodeObList", 42, hBasenode);
}

void ObjectBrowserControlExt::Redraw_Tunnel()
{
    HTREEITEM& hTunnel = ExtNodes[Root_Tunnel];
    if (hTunnel == NULL)   return;

    if (GlobalVars::INIFiles::FAData().GetBool("Debug", "AllowTunnels"))
    {
        this->InsertTranslatedString("NewTunnelObList", 50, hTunnel);
        this->InsertTranslatedString("DelTunnelObList", 51, hTunnel);
    }
}

void ObjectBrowserControlExt::Redraw_PlayerLocation()
{
    HTREEITEM& hPlayerLocation = ExtNodes[Root_PlayerLocation];
    if (hPlayerLocation == NULL)   return;

    this->InsertTranslatedString("StartpointsDelete", 21, hPlayerLocation);
}

int ObjectBrowserControlExt::GuessType(const char* pRegName)
{
    if (ExtSets[Set_Building].find(pRegName) != ExtSets[Set_Building].end())
        return Set_Building;
    if (ExtSets[Set_Infantry].find(pRegName) != ExtSets[Set_Infantry].end())
        return Set_Infantry;
    if (ExtSets[Set_Vehicle].find(pRegName) != ExtSets[Set_Vehicle].end())
        return Set_Vehicle;
    if (ExtSets[Set_Aircraft].find(pRegName) != ExtSets[Set_Aircraft].end())
        return Set_Aircraft;
    return -1;
}

int ObjectBrowserControlExt::GuessSide(const char* pRegName, int nType)
{
    auto& knownIterator = KnownItem.find(pRegName);
    if (knownIterator != KnownItem.end())
        return knownIterator->second;

    int result = -1;
    switch (nType)
    {
    case -1:
    default:
        break;
    case Set_Building:
        result = GuessBuildingSide(pRegName);
        break;
    case Set_Infantry:
        result = GuessGenericSide(pRegName, Set_Infantry);
        break;
    case Set_Vehicle:
        result = GuessGenericSide(pRegName, Set_Vehicle);
        break;
    case Set_Aircraft:
        result = GuessGenericSide(pRegName, Set_Aircraft);
        break;
    }
    KnownItem[pRegName] = result;
    return result;
}

int ObjectBrowserControlExt::GuessBuildingSide(const char* pRegName)
{
    auto& rules = GlobalVars::INIFiles::Rules();

    int planning;
    planning = rules.GetInteger(pRegName, "AIBasePlanningSide", -1);
    if (planning >= rules.GetKeyCount("Sides"))
        return -1;
    if (planning >= 0)
        return planning;
    auto& cons = STDHelpers::SplitString(rules.GetString("AI", "BuildConst"));
    int i;
    for (i = 0; i < cons.size(); ++i)
    {
        if (cons[i] == pRegName)
            return i;
    }
    if (i >= rules.GetKeyCount("Sides"))
        return -1;
    return GuessGenericSide(pRegName, Set_Building);
}

int ObjectBrowserControlExt::GuessGenericSide(const char* pRegName, int nType)
{
    auto& rules = GlobalVars::INIFiles::Rules();
    auto& set = ExtSets[nType];

    if (set.find(pRegName) == set.end())
        return -1;
    for (auto& prep : STDHelpers::SplitString(rules.GetString(pRegName, "Prerequisite")))
    {
        int guess = -1;
        for (auto& subprep : STDHelpers::SplitString(rules.GetString("GenericPrerequisites", prep)))
        {
            guess = GuessSide(subprep, GuessType(subprep));
            if (guess != -1)
                return guess;
        }
        guess = GuessSide(prep, GuessType(prep));
        if (guess != -1)
            return guess;
    }
    return -1;
}

// ObjectBrowserControlExt::OnSelectChanged

int ObjectBrowserControlExt::UpdateEngine(int nData)
{
    if (nData < 10000)
        switch (nData)
        {
        case 114514:
            return 0x51BB28;
        default:
            return 0x51AFBE;
        }
    else
        return 0x51BB28;
}