[ComponentEditorProps(category: "BEAR/Components", description: "Enables this Entity to be a functional Laser Pointer", color: "0 0 255 255")]
class BEAR_LaserPointerComponentClass: ScriptComponentClass
{
};

enum BEAR_LaserPointerDrawing {
	LIGHT, 
	MESH,
	DECAL
}

/// This component enables it's object to be a laser pointer. Simply call SetLaserEnabled(true/false);
class BEAR_LaserPointerComponent: ScriptComponent
{
	[RplProp(onRplName: "OnIsOnChanged")]
	protected bool _isOn = false;	
	
	protected Decal _decal;	
	protected SoundComponent _soundComponent;
	protected IEntity _laserDotEntity;
	protected IEntity _cachedPlayerOwningLaser;
	protected ref array<IEntity> _cachedIgnoreList;
	
	[Attribute(defvalue: "0 0 0", uiwidget: UIWidgets.Coords, desc: "Rotation of the laser in degrees (0,0,0 is default Z+)" )]
	vector LaserForwardsRotation;
	[Attribute("1", UIWidgets.Slider, desc: "Size of the Laser Dot", "0.1 5 0.001")]
	float LaserSize;
	
	// It seems like setting a color on a Material is currently not possible in workbench?
	//[Attribute("1 0.1 0.1 0", UIWidgets.ColorPicker, desc: "Color of the Laser Dot")]
	//ref Color LaserColor; 
	
	const BEAR_LaserPointerDrawing drawingType = BEAR_LaserPointerDrawing.MESH;
	
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
		_isOn = enabled;
		OnIsOnChanged();
		Replication.BumpMe();
	}
	
	protected void OnIsOnChanged()
	{
		if(_soundComponent) _soundComponent.SoundEvent("Sound_SwitchPressed");
		
		if(!_isOn && _decal)
		{
			World world = GetOwner().GetWorld();
			world.RemoveDecal(_decal);
			_decal = NULL;
		}
		if(!_isOn && _laserDotEntity)
		{
			delete _laserDotEntity;
		}
		
		BEAR_LaserPointerAction._isOn = _isOn;
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
		_cachedIgnoreList = {GetOwner()};
		
		SetEventMask(owner, EntityEvent.POSTFRAME);
	}
	
	// This method is called every frame.
	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		if(!_isOn) return;
		
		// Make sure the laser is not hitting the weapon or the PEQbox for example
		IEntity playerOwningLaser = GetPlayerOwningLaser();	
		if(playerOwningLaser != _cachedPlayerOwningLaser)
		{
			_cachedIgnoreList = {};
			CreateExcludeArray(playerOwningLaser, _cachedIgnoreList);
			_cachedPlayerOwningLaser = playerOwningLaser;
		}	
		
		vector position = owner.GetOrigin();
		vector direction = (owner.GetYawPitchRoll() + LaserForwardsRotation).AnglesToVector();
		
		// The Trace is a physics raycast that checks if any physics stuff is hit in a straight line
		TraceParam trace = CreateTrace(position, direction);
		trace.ExcludeArray = _cachedIgnoreList;
		World world = owner.GetWorld();
		float hitDistance = world.TraceMove(trace, null);
		vector hitPosition = vector.Direction(trace.Start, trace.End) * hitDistance + trace.Start;
		
		//Shape.CreateSphere(COLOR_RED, ShapeFlags., hitPosition, 1);
		
		if(drawingType == BEAR_LaserPointerDrawing.DECAL) 
			UpdateLaserDotDecal(hitPosition, trace);
		else if(drawingType == BEAR_LaserPointerDrawing.MESH)
			UpdateLaserDotEntity(hitPosition, trace);
		else if(drawingType == BEAR_LaserPointerDrawing.LIGHT)
			UpdateLaserLight(hitPosition, trace);
	}
	
	protected void UpdateLaserDotEntity(vector targetPosition, TraceParam trace)
	{	
		targetPosition = targetPosition + (trace.TraceNorm * 0.005);
		
		if(!_laserDotEntity)
		{
			// Not working :(
			/*ResourceName materialName = "{87F66F81E518E89D}Assets/LaserPointer/Visual/LaserPointerSphere.emat";
			Material material = Material.GetMaterial(materialName);
			material.SetParam("Emissive", LaserColor.PackToInt());*/
			
			EntitySpawnParams params = new EntitySpawnParams();
			params.Transform[3] = targetPosition;
			params.Scale = LaserSize;
			_laserDotEntity = GetGame().SpawnEntityPrefabLocal(Resource.Load("{5AE60D9669773AFE}Prefabs/LaserDot.et"), GetOwner().GetWorld(), params);
			
			// This flag makes it so the dot does not disappear when viewed from angles mildly different from the initial spawn position. 
			// Finding this took hours from my life which I will never get back :)
			_laserDotEntity.SetFlags(EntityFlags.ACTIVE, false);
		}
		
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
	
	protected void UpdateLaserLight(vector targetPosition, TraceParam trace)
	{
		// TODO: Find a way to make small lights draw at distance
		/*
		m_LightHandle = LightHandle.AddStaticLight(GetOwner().GetWorld(), LightType.POINT, lightFlags, m_fRadiusOfFlash, Color.FromVector(m_vCol), m_fLV, m_vOffset.Multiply4(mat));
		m_LightHandle.SetLensFlareType(entity.GetWorld(), LightLensFlareType.Disabled);
		m_LightHandle.SetIntensityEVClip(entity.GetWorld(), m_fEVClip);
		*/
	}
	
	protected IEntity GetPlayerOwningLaser()
	{
		IEntity weaponParent = GetOwner().GetParent();
		if(!weaponParent) return null;
		return weaponParent.GetParent();
	}
	
	// passes out an array of objects that should be ignored by the Physics Ray
	protected void CreateExcludeArray(IEntity playerOwningLaser, notnull out array<IEntity> result)
	{
		// Exclude the laser caster object
		result.Insert(GetOwner());
		
		// Exclude all of our weapons
		AddWeaponsOfPlayer(playerOwningLaser, result);
		
		// Exclude the player (hands were problematic)
		result.Insert(playerOwningLaser);
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
		trace.End = position + direction * 1000;
		
		// TODO: Find cleaner flags and mask values...
		trace.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		trace.LayerMask = EPhysicsLayerDefs.Projectile;
		
		return trace;
	}
};

/** @}*/
