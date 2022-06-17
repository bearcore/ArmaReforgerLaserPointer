[BaseContainerProps()]
class BEAR_WeaponHasLaserPointerCondition : SCR_AvailableActionCondition
{
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if(!data) return false;
		
		IEntity weapon = data.GetCurrentWeaponEntity();
		if(!weapon) return false;
		
		array<Managed> slots = {};
		weapon.FindComponents(AttachmentSlotComponent, slots);
		foreach(Managed slot : slots)
		{
			AttachmentSlotComponent slotComponent = AttachmentSlotComponent.Cast(slot);
			IEntity attachment = slotComponent.GetAttachedEntity();
			if(!attachment) continue;
			
			BEAR_LaserPointerComponent laser = BEAR_LaserPointerComponent.Cast(attachment.FindComponent(BEAR_LaserPointerComponent));
			if(laser) 
			{
				return GetReturnResult(true);
			}
		}
		
		return GetReturnResult(false);
	}
}