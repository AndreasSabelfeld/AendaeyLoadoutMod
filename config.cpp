class CfgPatches
{
    class AendaeyLoadoutMod
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = {"DZ_Data", "DZ_Scripts"};
    };
};

class CfgMods
{
    class AendaeyLoadoutMod
    {
        dir = "AendaeyLoadoutMod";
        name = "Aendaey Loadout Mod";
        type = "mod";
        dependencies[] = {"Game", "World", "Mission"};

        class defs
        {
            class gameScriptModule
            {
                value = "";
                files[] = {"AendaeyLoadoutMod/Scripts/3_Game"};
            };
            class worldScriptModule
            {
                value = "";
                files[] = {"AendaeyLoadoutMod/Scripts/4_World"};
            };
            class missionScriptModule
            {
                value = "";
                files[] = {"AendaeyLoadoutMod/Scripts/5_Mission"};
            };
        };
    };
};