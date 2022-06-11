[ComponentEditorProps(category: "BEAR/Components", description: "Enables this Entity to be a functional Laser Pointer", color: "0 0 255 255")]
class BEAR_LaserPointerComponentClass: ScriptComponentClass
{
};

/// This component enables it's object to be a laser pointer. Simply call SetLaserEnabled(true/false);
class BEAR_LaserPointerComponent: ScriptComponent
{
	protected bool _isOn = false;	
	protected Decal _decal;	
	protected SoundComponent _soundComponent;
	protected IEntity _laserDotEntity;
	
	const bool USE_DECAL = true;
	
	void ShowLaserUI()
	{
		// Not currently used. Work in Progress.
		BEAR_LaserPointerUI ui = BEAR_LaserPointerUI.Cast(GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.BEAR_LaserMenu)); 
		ui.SetActiveLaser(this);
	}
	
	void ToggleLaserEnabled()
	{
		SetLaserEnabled(!_isOn);
	}
	
	void SetLaserEnabled(bool enabled)
	{
		RpcBroadcastLaserEnabled(enabled);
		Rpc(RpcBroadcastLaserEnabled, enabled);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcBroadcastLaserEnabled(bool enabled)
	{
		if(_soundComponent) _soundComponent.SoundEvent("Sound_SwitchPressed");
		
		_isOn = enabled;
		if(!_isOn && _decal)
		{
			World world = GetOwner().GetWorld();
			world.RemoveDecal(_decal);
			_decal = NULL;
		}
		
		BEAR_LaserPointerAction._isOn = enabled;
	}
	
	// We need the OnPostInit and EOnInit for the EOnFrame to be called.
	protected override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}
	protected override void EOnInit(IEntity owner)
	{
		_soundComponent = SoundComponent.Cast(owner.FindComponent(SoundComponent));
		SetEventMask(owner, EntityEvent.POSTFRAME);
	}
	
	// This method is called every frame.
	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		if(!_isOn) return;
		
		// Make sure the laser is not hitting the weapon or the PEQbox for example
		array<IEntity> exclude = {};
		CreateExcludeArray(exclude);
		
		vector position = owner.GetOrigin();
		vector direction = owner.GetYawPitchRoll().AnglesToVector();
		
		// The Trace is a physics raycast that checks if any physics stuff is hit in a straight line
		TraceParam trace = CreateTrace(position, direction);
		trace.ExcludeArray = exclude;
		World world = owner.GetWorld();
		float hitDistance = world.TraceMove(trace, null);
		vector hitPosition = vector.Direction(trace.Start, trace.End) * hitDistance + trace.Start;
		
		if(USE_DECAL) 
			UpdateLaserDotDecal(hitPosition, trace);
		else 
			UpdateLaserDotEntity(hitPosition);
	}
	
	protected void UpdateLaserDotEntity(vector targetPosition)
	{	
		if(!_laserDotEntity)
			_laserDotEntity = GetGame().SpawnEntityPrefab(Resource.Load("{5AE60D9669773AFE}Prefabs/LaserDot.et"), GetOwner().GetWorld());
		_laserDotEntity.SetOrigin(targetPosition);
	}
	
	protected void UpdateLaserDotDecal(vector targetPosition, TraceParam trace)
	{
		World world = GetOwner().GetWorld();
		
		// Remove the decal from the previous frame
		if(_decal)
		{
			world.RemoveDecal(_decal);
			_decal = NULL;
		}
		
		// If we're aiming at the sky, don't make a decal
		if(!trace.TraceEnt) return;
		
		// The decal is what will actually display our laser. You can think of decals like spray cans. They color everything they hit in a radius.
		vector decalOrigin = targetPosition + (trace.TraceNorm * 0.2);
		ResourceName material = "{D53F4AD028D21A94}Assets/LaserPointerDecal.emat";
		float nearClip = 0;
		float farClip = 0.3;
		float angle = 0;
		float size = 0.5;
		float stretch = 1;
		float lifeTime = 0;
		int color = 0xFFFFFFFF;
		
		_decal = world.CreateDecal(trace.TraceEnt, decalOrigin, -trace.TraceNorm, 
			nearClip, farClip, angle * Math.DEG2RAD, size, stretch, material, lifeTime, color);
	}
	
	// passes out an array of objects that should be ignored by the Physics Ray
	protected void CreateExcludeArray(notnull out array<IEntity> result)
	{
		// Exclude the laser caster object
		result.Insert(GetOwner());
		
		IEntity playerOwningLaser = GetPlayerOwningLaser();
		if(!playerOwningLaser) return;
		
		// Exclude all of our weapons
		AddWeaponsOfPlayer(playerOwningLaser, result);
		
		// Exclude the player (hands were problematic)
		result.Insert(playerOwningLaser);
	}
	
	// Get the weapon that holds this attachment and then get the player holding that weapon.
	// I wish there was an easier way, something like InventoryItemComponent.GetPlayerOwner()?
	protected IEntity GetPlayerOwningLaser()
	{
		InventoryItemComponent attachment = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if(!attachment.GetParentSlot()) return null;
		BaseInventoryStorageComponent storage = attachment.GetParentSlot().GetStorage();
		WeaponComponent attachmentWeapon = WeaponComponent.Cast(storage.GetOwner().FindComponent(WeaponComponent));
		
		array<int> playerIds = {};
		GetGame().GetPlayerManager().GetPlayers(playerIds);
		foreach(int playerId : playerIds)
		{
			IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			if(!player) continue;
			BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(player.FindComponent(BaseWeaponManagerComponent));
		
			array<IEntity> weapons = {};
			weaponManager.GetWeaponsList(weapons);
			foreach(IEntity weapon : weapons)
			{
				if(weapon == attachmentWeapon.GetOwner())
				{
					return player;
				}
			}
		}
		
		return null;
	}
	
	// Adds the weapons the passed player is holding
	protected void AddWeaponsOfPlayer(IEntity player, out array<IEntity> result)
	{
		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(player.FindComponent(BaseWeaponManagerComponent));
		array<IEntity> weapons = {};
		weaponManager.GetWeaponsList(weapons);
		foreach(IEntity weapon: weapons)
		{
			result.Insert(weapon);
		}
	}
	
	// Creates the settigns for a Physics Ray to be shot from the Laser origin
	protected TraceParam CreateTrace(vector position, vector direction)
	{
		TraceParam trace = new TraceParam();
		trace.Start = position;
		trace.End = position + direction * 100;
		
		// TODO: Find cleaner flags and mask values...
		trace.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		trace.LayerMask = EPhysicsLayerDefs.Projectile;
		
		return trace;
	}
};

/** @}*/
