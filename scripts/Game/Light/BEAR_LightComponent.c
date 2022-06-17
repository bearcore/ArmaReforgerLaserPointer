[ComponentEditorProps(category: "BEAR/Components", description: "Enables this Entity to be a functional Weapon Light", color: "0 0 255 255")]
class BEAR_LightComponentClass: ScriptComponentClass
{
};

/// This component enables it's object to be a light. Simply call SetLightEnabled(true/false);
class BEAR_LightComponent: ScriptComponent
{
	[RplProp(onRplName: "OnIsOnChanged")]
	protected bool _isOn = false;	
	
	protected SoundComponent _soundComponent;
	protected IEntity _cachedPlayerOwningLight;
	protected IEntity _lightOwner;
	protected ref array<IEntity> _cachedIgnoreList;
	protected LightHandle _lightHandle;
	
	private Shape _debugShape;
	
	[Attribute(defvalue: "0 0 0", uiwidget: UIWidgets.Coords, desc: "Offset of the Light Origin from the Entity Origin" )]
	vector LightOffset;
	[Attribute(defvalue: "0 0 0", uiwidget: UIWidgets.Coords, desc: "Rotation of the light in degrees (0,0,0 is default Z+)" )]
	vector LightForwardsRotation;
	[Attribute("false", UIWidgets.CheckBox, desc: "Show Debug Arrow to visualize the Light")]
	bool DebugLightOffset;
	[Attribute("100", UIWidgets.Slider, desc: "Size of the light ? I'm not sure", "0 100 0.1")]
	float LightSize;
	[Attribute("50", UIWidgets.Slider, desc: "Size of the light Cone", "0 100 0.1")]
	float LightCone;
	[Attribute("1 1 1 1", UIWidgets.ColorPicker, desc: "Color of the Light")]
	ref Color LightColor; 
	[Attribute("4", UIWidgets.Slider, desc: "Brightness of the Light (Bloom essentially)", "0 20 0.1")]
	float LightBrightness;
	[Attribute("0.1", UIWidgets.Slider, desc: "The distance from where shadow casting starts.", "0.01 2 0.001")]
	float NearPlane;
	
	void ToggleLightEnabled()
	{
		SetLightEnabled(!_isOn);
	}
	
	void SetLightEnabled(bool enabled)
	{
		_isOn = enabled;
		OnIsOnChanged();
		Replication.BumpMe();
	}
	
	protected void OnIsOnChanged()
	{
		if(_soundComponent) _soundComponent.SoundEvent("Sound_SwitchPressed");
		
		if(!_isOn && _lightHandle)
		{
			_lightHandle.RemoveLight(GetOwner().GetWorld());
			_lightHandle = NULL;
			delete _lightOwner;
		}
		
		BEAR_LightAction._isOn = _isOn;
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
		
		GetGame().GetInputManager().AddActionListener("LightPointer", EActionTrigger.DOWN, KeybindPressed);
		
		SetEventMask(owner, EntityEvent.POSTFRAME);
	}
	
	private void KeybindPressed()
	{
		IEntity localPlayer = GetGame().GetPlayerController().GetControlledEntity();
		
		if(localPlayer == GetPlayerOwningLight())
			ToggleLightEnabled();
	}
	
	// This method is called every frame.
	override void EOnPostFrame(IEntity owner, float timeSlice)
	{
		if(_debugShape) delete _debugShape;
		if(!_isOn) return;
		
		// Make sure the laser is not hitting the weapon or the PEQbox for example
		IEntity playerOwningLight = GetPlayerOwningLight();	
		if(playerOwningLight != _cachedPlayerOwningLight)
		{
			_cachedIgnoreList = {};
			CreateExcludeArray(playerOwningLight, _cachedIgnoreList);
			_cachedPlayerOwningLight = playerOwningLight;
		}	
		
		vector direction = owner.GetYawPitchRoll() + LightForwardsRotation;
		vector transform[4];
		owner.GetWorldTransform(transform);
		vector position = owner.GetOrigin() + LightOffset.Multiply3(transform);
		
		if(DebugLightOffset)
			_debugShape = Shape.CreateArrow(position, position + direction.AnglesToVector() * 0.1, 0.04, Color.Red.PackToInt(), ShapeFlags.NOZBUFFER);
		
		UpdateLight(owner, position, direction);
	}
	
	protected void UpdateLight(IEntity owner, vector position, vector direction)
	{	
		auto world = owner.GetWorld();
		/*vector transform[4];
		owner.GetTransform(transform);
		transform*/
		
		if(!_lightHandle)
		{
			EntitySpawnParams spawnParams = new EntitySpawnParams();
			spawnParams.Parent = owner;
			spawnParams.TransformMode = ETransformMode.OFFSET;
			_lightOwner = GetGame().SpawnEntity(GenericEntity, world, spawnParams);
			_lightOwner.SetOrigin(position);
			_lightOwner.SetAngles(direction);
			
			_lightHandle = LightHandle.AddDynamicLight(_lightOwner, LightType.SPOT, LightFlags.CASTSHADOW, LightSize, LightColor, LightBrightness);
			_lightHandle.SetLensFlareType(world, LightLensFlareType.Automatic);
			_lightHandle.SetCone(world, LightCone);
			_lightHandle.SetNearPlane(world, NearPlane);
		}
		
		_lightOwner.SetOrigin(position);
		vector inverse = {direction[1], direction[0], 0};
		_lightOwner.SetAngles(inverse);
		//_lightHandle.SetDirection(world, direction);
	}
	
	protected IEntity GetPlayerOwningLight()
	{
		IEntity weaponParent = GetOwner().GetParent();
		if(!weaponParent) return null;
		return weaponParent.GetParent();
	}
	
	// passes out an array of objects that should be ignored by the Light
	protected void CreateExcludeArray(IEntity playerOwningLaser, notnull out array<IEntity> result)
	{
		// Exclude the light caster object
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
};
