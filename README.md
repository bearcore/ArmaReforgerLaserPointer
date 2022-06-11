# ArmaReforgerLaserPointer

How to add to your weapon mod:

1. Download the mod from the ingame workshop

3. Add 598F2330524F15B8 to your mod dependencies 
<img width="400" alt="ArmaReforgerWorkbench_YivIQXXCPc" src="https://user-images.githubusercontent.com/62651909/173182617-8b72d7e7-25dc-4a5d-8a0f-e9c64a69e704.png">
<img width="600" alt="ArmaReforgerWorkbench_5kWW1ySNEK" src="https://user-images.githubusercontent.com/62651909/173182619-deadcea2-99d5-4589-82cb-8996d7b27fb5.png">

3. Restart the editor


Now you can check out how the example laserpointer works: Prefabs\Weapons\Attachments\Handguards\PEQ15\PEQ15.et

To add the functionality to your existing laserpointer, simply add a BEAR_LaserPointerComponent to it.
Additionally, you can add a SoundComponent and add the LaserPointer.acp in the Filenames field to get the clicky sound when toggling the laser.

<img width="456" alt="ArmaReforgerWorkbench_7l25krmpub" src="https://user-images.githubusercontent.com/62651909/173182773-2bc722bd-5a18-4e5e-a5f8-59f014f7377e.png">


Alternatively, simply duplicate the PEQ15.et, open your duplicate and then change the Object in the MeshObject Component to give it your own model.
