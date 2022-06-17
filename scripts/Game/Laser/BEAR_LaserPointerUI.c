modded enum ChimeraMenuPreset {
    BEAR_LaserMenu
}

class BEAR_LaserPointerUI : ChimeraMenuBase
{	
	protected ItemPreviewWidget m_wLaserRender;
	protected BEAR_LaserPointerComponent m_Laser;
	protected ItemPreviewManagerEntity 	m_pPreviewManager;
	protected Widget m_widget;
	protected ref PreviewRenderAttributes _laserRenderAttributes
	private InputManager m_pInputmanager;
	
	const vector _limitsMin = "-180 -180 -180";
	const vector _limitsMax = "180 180 180";
	const float _zoomMin = 15;
	const float _zoomMax = 125;
	const float _mouseSpeed = 0.01;
	
	protected float _time;
	protected float _lastFrameX;
	protected float _lastFrameY;
	
	override void OnMenuOpen()
	{
 		m_pInputmanager = GetGame().GetInputManager();
				
		m_widget = GetRootWidget();
		m_pPreviewManager = GetGame().GetItemPreviewManager();
		m_wLaserRender = ItemPreviewWidget.Cast(m_widget.FindAnyWidget("laserRender"));
		
		_laserRenderAttributes = new PreviewRenderAttributes();
		_laserRenderAttributes.RotateItemCamera("-90 -90 -90", _limitsMin, _limitsMax);
		_laserRenderAttributes.ZoomCamera(-35, _zoomMin, _zoomMax);
		
		float resX; float resY; 
		GetGame().GetWorkspace().GetScreenSize(resX, resY);
		_lastFrameX = resX / 2;
		_lastFrameY = resY / 2;
	}
	
	override  bool OnClick(Widget w, int x, int y, int button)
	{
		if(w.GetName() == "CloseButton")
		{
			GetGame().GetMenuManager().CloseMenu(this);
			return true;
		}
		m_Laser.ToggleLaserEnabled();
		return true;
	}
	
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		
		_time += tDelta;
		UpdateInputs(tDelta);
	}
	
	protected void UpdateInputs(float deltaTime)
	{
		float newX = m_pInputmanager.GetActionValue("MouseX");
		float newY = m_pInputmanager.GetActionValue("MouseY");
		
		float xDiff = newX - _lastFrameX;
		float yDiff = newY - _lastFrameY;
		vector _thisisstupid;
		_thisisstupid[0] = xDiff;
		_thisisstupid[1] = yDiff * 0.2;
		_laserRenderAttributes.RotateItemCamera(_thisisstupid * _mouseSpeed, _limitsMin, _limitsMax);
		
		_lastFrameX = newX;
		_lastFrameY = newY;
		
		RefreshLaserWidget();
	}

	protected void RefreshLaserWidget()
	{		
		if(!m_Laser) return;
		
		m_pPreviewManager.SetPreviewItem(m_wLaserRender, GenericEntity.Cast(m_Laser.GetOwner()), _laserRenderAttributes, true);
		m_wLaserRender.SetResolutionScale(1, 1);
	}
	
	void SetActiveLaser(BEAR_LaserPointerComponent laser)
	{
		m_Laser = laser;
		RefreshLaserWidget();
	}
}