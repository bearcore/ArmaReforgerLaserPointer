[BaseContainerProps()]
class BEAR_WeaponHasLightPointerCondition : SCR_AvailableActionCondition
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
			
			BEAR_LightComponent light = BEAR_LightComponent.Cast(attachment.FindComponent(BEAR_LightComponent));
			if(light) 
			{
				return GetReturnResult(true);
			}
		}
		
		return GetReturnResult(false);
	}
}