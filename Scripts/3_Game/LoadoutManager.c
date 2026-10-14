enum AendaeyLoadoutRPC
{
    RPC_REQUEST_LOADOUTS = 9997,
    RPC_SYNC_LOADOUTS    = 9998,
    RPC_APPLY_LOADOUT    = 9999
};

class LoadoutConfig : Managed
{
    string LoadoutName;
    string Description;
    ref array<string> Clothes;
    ref array<string> Weapons;
    ref array<string> PrimaryAttachments;
    string PrimaryMag;
    string PrimaryAmmo;
    string SecondaryMag;

    void LoadoutConfig()
    {
        Description = "";
        Clothes = new array<string>();
        Weapons = new array<string>();
        PrimaryAttachments = new array<string>();
        PrimaryMag = "";
        PrimaryAmmo = "";
        SecondaryMag = "";
    }
}

class AendaeyLoadoutManager
{
    static const string LOADOUT_FOLDER = "$profile:AendaeyLoadouts";
    static const string LOADOUT_DIR    = "$profile:AendaeyLoadouts/";
    static ref array<ref LoadoutConfig> m_AllLoadouts = new array<ref LoadoutConfig>();
    static bool m_ClientHasSynced = false;

    static void Log(string msg)
    {
        Print("[AendaeyLoadoutMod] " + msg);
    }

    static void LoadLoadouts()
    {
        m_AllLoadouts.Clear();

        if (!FileExist(LOADOUT_FOLDER))
        {
            Log("Directory does not exist, creating: " + LOADOUT_FOLDER);
            MakeDirectory(LOADOUT_FOLDER);
            CreateDefaultLoadouts();
        }

        string fileName;
        FileAttr fileAttr;
        string pattern = LOADOUT_DIR + "*.json";

        FindFileHandle handle = FindFile(pattern, fileName, fileAttr, FindFileFlags.ALL);
        if (handle)
        {
            if (!(fileAttr & FileAttr.DIRECTORY))
            {
                LoadSingleFile(LOADOUT_DIR + fileName);
            }

            while (FindNextFile(handle, fileName, fileAttr))
            {
                if (!(fileAttr & FileAttr.DIRECTORY))
                {
                    LoadSingleFile(LOADOUT_DIR + fileName);
                }
            }

            CloseFindFile(handle);
        }

        if (m_AllLoadouts.Count() == 0)
        {
            Log("No .json files found -> Creating all 15 default loadouts...");
            CreateDefaultLoadouts();

            handle = FindFile(pattern, fileName, fileAttr, FindFileFlags.ALL);
            if (handle)
            {
                if (!(fileAttr & FileAttr.DIRECTORY))
                    LoadSingleFile(LOADOUT_DIR + fileName);

                while (FindNextFile(handle, fileName, fileAttr))
                {
                    if (!(fileAttr & FileAttr.DIRECTORY))
                        LoadSingleFile(LOADOUT_DIR + fileName);
                }
                CloseFindFile(handle);
            }
        }

        Log(string.Format("Total of %1 loadouts loaded on server.", m_AllLoadouts.Count()));
    }

    protected static void LoadSingleFile(string fullPath)
    {
        LoadoutConfig config = new LoadoutConfig();
        string errorMsg;

        if (JsonFileLoader<LoadoutConfig>.LoadFile(fullPath, config, errorMsg))
        {
            m_AllLoadouts.Insert(config);
            Log(string.Format("JSON loaded: '%1' (Clothing: %2, Weapons: %3)", config.LoadoutName, config.Clothes.Count(), config.Weapons.Count()));
        }
        else
        {
            Log("ERROR while loading " + fullPath + ": " + errorMsg);
        }
    }

    static void WriteLoadoutsToRPC(ScriptRPC rpc)
    {
        int count = m_AllLoadouts.Count();
        rpc.Write(count);

        for (int i = 0; i < count; i++)
        {
            LoadoutConfig cfg = m_AllLoadouts[i];
            rpc.Write(cfg.LoadoutName);
            rpc.Write(cfg.Description);
            rpc.Write(cfg.Clothes);
            rpc.Write(cfg.Weapons);
            rpc.Write(cfg.PrimaryAttachments);
            rpc.Write(cfg.PrimaryMag);
            rpc.Write(cfg.PrimaryAmmo);
            rpc.Write(cfg.SecondaryMag);
        }
    }

    static bool ReadLoadoutsFromRPC(ParamsReadContext ctx)
    {
        int count = 0;
        if (!ctx.Read(count))
        {
            Log("ERROR: Could not read loadout count from RPC!");
            return false;
        }

        m_AllLoadouts.Clear();

        for (int i = 0; i < count; i++)
        {
            string name;
            string desc;
            array<string> clothes = new array<string>();
            array<string> weapons = new array<string>();
            array<string> primAtt = new array<string>();
            string primMag;
            string primAmmo;
            string secMag;

            if (!ctx.Read(name)) return false;
            if (!ctx.Read(desc)) return false;
            if (!ctx.Read(clothes)) return false;
            if (!ctx.Read(weapons)) return false;
            if (!ctx.Read(primAtt)) return false;
            if (!ctx.Read(primMag)) return false;
            if (!ctx.Read(primAmmo)) return false;
            if (!ctx.Read(secMag)) return false;

            LoadoutConfig cfg = new LoadoutConfig();
            cfg.LoadoutName = name;
            cfg.Description = desc;
            cfg.Clothes.Copy(clothes);
            cfg.Weapons.Copy(weapons);
            cfg.PrimaryAttachments.Copy(primAtt);
            cfg.PrimaryMag = primMag;
            cfg.PrimaryAmmo = primAmmo;
            cfg.SecondaryMag = secMag;

            m_AllLoadouts.Insert(cfg);
            Log(string.Format("Client has received loadout #%1: '%2'", i + 1, cfg.LoadoutName));
        }

        m_ClientHasSynced = true;
        Log(string.Format("Client-Sync completed: %1 loadouts available.", m_AllLoadouts.Count()));
        return true;
    }

    protected static LoadoutConfig MakeTacticalPreset(string name, string desc, string primaryWpn, string secondaryWpn, array<string> attachments, string primMag, string primAmmo, string secMag)
    {
        LoadoutConfig cfg = new LoadoutConfig();
        cfg.LoadoutName = name;
        cfg.Description = desc;

        cfg.Clothes.Insert("CargoPants_Black");
        cfg.Clothes.Insert("TacticalShirt_Black");
        cfg.Clothes.Insert("CombatBoots_Black");
        cfg.Clothes.Insert("TacticalGloves_Black");
        cfg.Clothes.Insert("AssaultBag_Black");
        cfg.Clothes.Insert("Mich2001Helmet");
        cfg.Clothes.Insert("PlateCarrierVest");

        cfg.Weapons.Insert(primaryWpn);
        cfg.Weapons.Insert(secondaryWpn);

        if (attachments)
        {
            cfg.PrimaryAttachments.Copy(attachments);
        }

        cfg.PrimaryMag = primMag;
        cfg.PrimaryAmmo = primAmmo;
        cfg.SecondaryMag = secMag;
        return cfg;
    }

    protected static void CreateDefaultLoadouts()
    {
        string err;

        JsonFileLoader<LoadoutConfig>.SaveFile(LOADOUT_DIR + "01_M4A1.json", MakeTacticalPreset("M4A1 Assault", "US-Sturmgewehr (5.56x45mm) mit ACOG Visier & 30er STANAG-Magazinen.", "M4A1", "FNX45", {"M4_OEBttstck", "M4_PlasticHndgrd", "ACOGOptic"}, "Mag_STANAG_30Rnd", "", "Mag_FNX45_15Rnd"), err);
        JsonFileLoader<LoadoutConfig>.SaveFile(LOADOUT_DIR + "02_AKM.json", MakeTacticalPreset("AKM Sturmgewehr", "Schweres 7.62x39mm Sturmgewehr mit Kobra-Visier und hoher Durchschlagskraft.", "AKM", "Glock19", {"AK_WoodBttstck", "AK_WoodHndgrd", "KobraOptic"}, "Mag_AKM_30Rnd", "", "Mag_Glock_15Rnd"), err);
        JsonFileLoader<LoadoutConfig>.SaveFile(LOADOUT_DIR + "03_AK74.json", MakeTacticalPreset("AK-74 Infanterie", "Präzises 5.45x39mm Sturmgewehr mit geringem Rückstoß und Kobra-Visier.", "AK74", "FNX45", {"AK74_WoodBttstck", "AK74_WoodHndgrd", "KobraOptic"}, "Mag_AK74_30Rnd", "", "Mag_FNX45_15Rnd"), err);
        JsonFileLoader<LoadoutConfig>.SaveFile(LOADOUT_DIR + "04_FAL.json", MakeTacticalPreset("LAR (FN FAL)", "Schweres .308 Win Kampfgewehr mit ACOG-Optik für mittlere und weite Distanz.", "FAL", "FNX45", {"FAL_OeBttstck", "ACOGOptic"}, "Mag_FAL_20Rnd", "", "Mag_FNX45_15Rnd"), err);
        JsonFileLoader<LoadoutConfig>.SaveFile(LOADOUT_DIR + "05_SVD.json", MakeTacticalPreset("VSD (SVD Dragunov)", "Halbautomatisches 7.62x54mmR Scharfschützengewehr mit PSO-1 Zielfernrohr.", "SVD", "Glock19", {"PSO1Optic"}, "Mag_SVD_10Rnd", "", "Mag_Glock_15Rnd"), err);
        JsonFileLoader<LoadoutConfig>.SaveFile(LOADOUT_DIR + "06_Mosin9130.json", MakeTacticalPreset("Mosin 91/30 Sniper", "Klassisches 7.62x54mmR Repetiergewehr mit PU-Zielfernrohr.", "Mosin9130", "FNX45", {"PUScopeOptic"}, "", "Ammo_762x54", "Mag_FNX45_15Rnd"), err);
        JsonFileLoader<LoadoutConfig>.SaveFile(LOADOUT_DIR + "07_SKS.json", MakeTacticalPreset("SK 59/66 (SKS)", "Halbautomatischer 7.62x39mm Karabiner mit PU-Zielfernrohr.", "SKS", "Glock19", {"PUScopeOptic"}, "", "Ammo_762x39", "Mag_Glock_15Rnd"), err);
        JsonFileLoader<LoadoutConfig>.SaveFile(LOADOUT_DIR + "08_Winchester70.json", MakeTacticalPreset("M70 Tundra Sniper", "Präzisions-Jagdgewehr im Kaliber .308 Win mit 12x Hunting-Scope.", "Winchester70", "FNX45", {"HuntingOptic"}, "", "Ammo_308Win", "Mag_FNX45_15Rnd"), err);
        JsonFileLoader<LoadoutConfig>.SaveFile(LOADOUT_DIR + "09_B95.json", MakeTacticalPreset("Blaze (B95)", "Doppelläufiges .308 Win Gewehr mit Hunting-Scope und Doppelschuss-Modus.", "B95", "Glock19", {"HuntingOptic"}, "", "Ammo_308Win", "Mag_Glock_15Rnd"), err);
        JsonFileLoader<LoadoutConfig>.SaveFile(LOADOUT_DIR + "10_ASVAL.json", MakeTacticalPreset("AS VAL SpecOps", "Schallgedämpftes 9x39mm Spezialgewehr mit 20-Schuss-Magazinen.", "ASVAL", "FNX45", {"KobraOptic"}, "Mag_VAL_20Rnd", "", "Mag_FNX45_15Rnd"), err);
        JsonFileLoader<LoadoutConfig>.SaveFile(LOADOUT_DIR + "11_CZ527.json", MakeTacticalPreset("CR-527 Scharfschütze", "Leichtes 7.62x39mm Repetiergewehr mit 5-Schuss-Magazinen und Hunting-Scope.", "CZ527", "Glock19", {"HuntingOptic"}, "Mag_CZ527_5rnd", "", "Mag_Glock_15Rnd"), err);
        JsonFileLoader<LoadoutConfig>.SaveFile(LOADOUT_DIR + "12_Saiga.json", MakeTacticalPreset("Vaiga (Saiga-12K)", "Vollautomatische 12-Gauge Schrotflinte mit 8-Schuss-Magazinen für den Nahkampf.", "Saiga", "FNX45", {"Saiga_Bttstck", "KobraOptic"}, "Mag_Saiga_8Rnd", "", "Mag_FNX45_15Rnd"), err);
        JsonFileLoader<LoadoutConfig>.SaveFile(LOADOUT_DIR + "13_Famas.json", MakeTacticalPreset("LE-MAS (FAMAS)", "Bullpup-Sturmgewehr (5.56x45mm) mit extrem hoher Feuerrate für den Nahkampf.", "Famas", "Glock19", {}, "Mag_FAMAS_25Rnd", "", "Mag_Glock_15Rnd"), err);
        JsonFileLoader<LoadoutConfig>.SaveFile(LOADOUT_DIR + "14_Aug.json", MakeTacticalPreset("AUR A1 (Steyr AUG)", "Kompaktes Bullpup-Sturmgewehr (5.56x45mm) mit integriertem 1.5x Visier.", "AugShort", "FNX45", {}, "Mag_STANAG_30Rnd", "", "Mag_FNX45_15Rnd"), err);
        JsonFileLoader<LoadoutConfig>.SaveFile(LOADOUT_DIR + "15_SVK.json", MakeTacticalPreset("VS-89 / SVK", "Modernes Scharfschützengewehr mit PSO-1-1 Optik und 10-Schuss-Magazinen.", "SVK", "Glock19", {"PSO11Optic"}, "Mag_SVK_10Rnd", "", "Mag_Glock_15Rnd"), err);
    }
}