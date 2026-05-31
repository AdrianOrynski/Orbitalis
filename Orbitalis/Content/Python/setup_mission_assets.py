"""
Orbitalis – Mission Asset Setup Script
Run from UE5 Output Log (switch to Python) or Tools > Execute Python Script.

Creates:
  /Game/Inputs/IA_Cam_Chase
  /Game/Inputs/IA_Cam_Overview
  /Game/Inputs/IA_Cam_Docking
  /Game/Inputs/IA_Restart
  /Game/Blueprints/BP_SpaceStation
  /Game/Blueprints/BP_MissionManager

Then adds A/S/D/R mappings to IMC_Spacecraft.
(F1/F2/F3 conflict with UE5 editor viewport shading modes.)
"""

import unreal

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
eal = unreal.EditorAssetLibrary


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def dup_ia(source_name, dest_name, folder='/Game/Inputs'):
    """Duplicate an existing IA asset (preserves ValueType=Boolean)."""
    src  = f'{folder}/{source_name}'
    dest = f'{folder}/{dest_name}'
    if eal.does_asset_exist(dest):
        unreal.log(f'[skip] {dest} already exists')
        return unreal.load_asset(dest)
    ok = eal.duplicate_asset(src, dest)
    if ok:
        eal.save_asset(dest)
        unreal.log(f'[OK]   created {dest}')
    else:
        unreal.log_error(f'[FAIL] could not create {dest}')
    return unreal.load_asset(dest) if ok else None


def make_blueprint(name, parent_class_path, folder='/Game/Blueprints'):
    """Create a Blueprint from a C++ parent class."""
    dest = f'{folder}/{name}'
    if eal.does_asset_exist(dest):
        unreal.log(f'[skip] {dest} already exists')
        return unreal.load_asset(dest)
    parent = unreal.load_class(None, parent_class_path)
    if parent is None:
        unreal.log_error(f'[FAIL] class not found: {parent_class_path}')
        return None
    factory = unreal.BlueprintFactory()
    factory.set_editor_property('parent_class', parent)
    asset = asset_tools.create_asset(name, folder, unreal.Blueprint, factory)
    if asset:
        eal.save_asset(dest)
        unreal.log(f'[OK]   created {dest}')
    else:
        unreal.log_error(f'[FAIL] could not create {dest}')
    return asset


STALE_KEYS = {'F1', 'F2', 'F3'}  # old bindings to remove if present

def add_imc_mapping(imc, action_asset, key_name):
    """Add or update a key -> action mapping in an IMC.
    Removes any old mapping for the same action before adding the new one."""
    mappings = list(imc.get_editor_property('mappings'))

    # Remove stale mappings for this action (e.g. old F1/F2/F3)
    mappings = [m for m in mappings if m.get_editor_property('action') != action_asset]

    mapping = unreal.EnhancedActionKeyMapping()
    mapping.set_editor_property('action', action_asset)

    key = unreal.Key()
    key.set_editor_property('key_name', key_name)
    mapping.set_editor_property('key', key)

    mappings.append(mapping)
    imc.set_editor_property('mappings', mappings)
    unreal.log(f'[OK]   IMC: {key_name} -> {action_asset.get_name()}')


# ---------------------------------------------------------------------------
# 1. Input Actions  (copy from IA_Main which is also Boolean/Triggered)
# ---------------------------------------------------------------------------

unreal.log('=== Creating Input Actions ===')
ia_chase    = dup_ia('IA_Main', 'IA_Cam_Chase')
ia_overview = dup_ia('IA_Main', 'IA_Cam_Overview')
ia_docking  = dup_ia('IA_Main', 'IA_Cam_Docking')
ia_restart  = dup_ia('IA_Main', 'IA_Restart')

# ---------------------------------------------------------------------------
# 2. Blueprints
# ---------------------------------------------------------------------------

unreal.log('=== Creating Blueprints ===')
make_blueprint('BP_SpaceStation',  '/Script/Orbitalis.SpaceStation')
make_blueprint('BP_MissionManager', '/Script/Orbitalis.MissionManager')

# ---------------------------------------------------------------------------
# 3. IMC_Spacecraft – A / S / D / R  (F1-F3 removed, conflict with editor)
# ---------------------------------------------------------------------------

unreal.log('=== Updating IMC_Spacecraft ===')
imc = unreal.load_asset('/Game/Inputs/IMC_Spacecraft')
if imc is None:
    unreal.log_error('[FAIL] IMC_Spacecraft not found')
else:
    if ia_chase:
        add_imc_mapping(imc, ia_chase,    'A')
    if ia_overview:
        add_imc_mapping(imc, ia_overview, 'S')
    if ia_docking:
        add_imc_mapping(imc, ia_docking,  'D')
    if ia_restart:
        add_imc_mapping(imc, ia_restart,  'R')
    eal.save_asset('/Game/Inputs/IMC_Spacecraft')

unreal.log('=== setup_mission_assets.py DONE ===')
unreal.log('Next steps:')
unreal.log('  1. Compile C++ (Build in VS or click Compile in editor)')
unreal.log('  2. Open BP_SpacecraftPawn -> assign IA_Cam_Chase/Overview/Docking in Details')
unreal.log('  3. Open BP_MissionManager -> assign IA_Restart in Details')
unreal.log('  4. Drag BP_SpaceStation + BP_MissionManager into the level')
unreal.log('  5. Assign a Static Mesh to BP_SpaceStation')
