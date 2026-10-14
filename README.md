# Aendaey Loadout Mod for DayZ

A lightweight and robust loadout selection menu for DayZ servers, allowing players to choose their starting gear smoothly upon spawning.

---

## Features

* **Dynamic UI Menu:** Automatically populates a clean selection interface based on available loadouts.
* **Scrollable List & Descriptions:** Supports custom descriptions and automatically lists all included weapons and equipment.
* **Smart Item Cleanup & Setup:** Fully equips players with clothing, helmets (with night-vision goggles and batteries pre-attached), plate carriers with pouches, medical items, and grenades.
* **Automatic JSON Generation:** If no loadout files exist on the server, default presets are automatically created in `$profile:AendaeyLoadouts/`.
* **DayZ 1.29 Ready:** Includes native weapon FSM synchronization to prevent weapon glitches after server join.

---

## Installation

1. Place the `@AendaeyLoadoutMod` folder in your DayZ server root directory.
2. Add `-mod=@AendaeyLoadoutMod` to your server startup parameters.
3. Configure your loadouts as JSON files in the server's profile folder (see [Configuration](#configuration)).

---

## Configuration

Loadout configuration files are stored in the server profile directory under:
`$profile:AendaeyLoadouts/`

You can create custom `.json` files for your server. Here is an example (`AssaultLoadout.json`):

```json
{
    "LoadoutName": "Assault",
    "Description": "Standard assault loadout for medium and close range combat.",
    "Clothes": [
        "CargoPants_Black",
        "TacticalShirt_Black",
        "CombatBoots_Black",
        "TacticalGloves_Black",
        "AssaultBag_Black",
        "Mich2001Helmet",
        "PlateCarrierVest"
    ],
    "Weapons": [
        "M4A1",
        "Glock19"
    ],
    "PrimaryAttachments": [
        "M4_OEBttstck",
        "M4_PlasticHndgrd",
        "ACOGOptic"
    ],
    "PrimaryMag": "Mag_STANAG_30Rnd",
    "PrimaryAmmo": "",
    "SecondaryMag": "Mag_Glock_15Rnd"
}