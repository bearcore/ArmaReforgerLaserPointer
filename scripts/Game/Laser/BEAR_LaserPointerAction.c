// This action is added to Weapon_Base.et, so it will always be on any weapon in the game.
class BEAR_LaserPointerAction : ScriptedUserAction
{
	static bool _isOn = false;
	
	// These 2 overrides and their return values ensure, that only the server calls PerformAction.
    override event bool HasLocalEffectOnlyScript() { return false; };
    override event bool CanBroadcastScript() { return false; };
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		//_isOn = false;
	}
	
	// As the BEAR_LaserPointerAction is on the Weapon_Base.et, make sure we have a laser pointer attached before showing the Action.
	// I don't like this solution because it will check constantly, even with no laser attached...
	// A cleaner solution would be to have the BEAR_LaserPointerComponent add an Action to the weapon, but the engine does not support adding Actions.
	override bool CanBeShownScript(IEntity user)
	{
		IEntity owner = GetOwner();
		
		array<Managed> slots = {};
		owner.FindComponents(AttachmentSlotComponent, slots);
		foreach(Managed slot : slots)
		{
			AttachmentSlotComponent slotComponent = AttachmentSlotComponent.Cast(slot);
			IEntity attachment = slotComponent.GetAttachedEntity();
			if(!attachment) continue;
			
			BEAR_LaserPointerComponent laser = BEAR_LaserPointerComponent.Cast(attachment.FindComponent(BEAR_LaserPointerComponent));
			if(laser) 
			{
				return true;
			}
		}
		
		return false;
	}
	
	override bool GetActionNameScript(out string outName)
	{
		if(_isOn)
			outName = "Disable Laser Pointer";
		else
			outName = "Enable Laser Pointer";
		return true;
	}
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		_isOn = !_isOn;
		
		array<Managed> slots = {};
		pOwnerEntity.FindComponents(AttachmentSlotComponent, slots);
		foreach(Managed slot : slots)
		{
			AttachmentSlotComponent slotComponent = AttachmentSlotComponent.Cast(slot);
			IEntity attachment = slotComponent.GetAttachedEntity();
			if(!attachment) continue;
			
			BEAR_LaserPointerComponent laser = BEAR_LaserPointerComponent.Cast(attachment.FindComponent(BEAR_LaserPointerComponent));
			if(laser) 
			{
				laser.SetLaserEnabled(_isOn);
			}
		}
	}
}