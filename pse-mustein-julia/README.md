# pse-mustein-julia

This example project demonstrates how to implement a model of a peripheral and making 
own custom platform inside Renode. 

For brevity, a fictional non-existent frame buffer graphics card was chosen
with very simplified register mapping as the real devices are much more complex.

Note: At the moment there is no functional HW equivalent for this example. 
The example is a Renode-only project mainly to demonstrate the modeling 
capabilities of Renode and how custom platforms can be made/tweaked. Use
the code in this example at your own risk.

Platform `/renode/pse-mustein-julia.resc` is attaching *Video.MusteinGenericGPU*
model at arbitrary memory location 0x10100000, if this value needs to be changed
then the application needs to match the new memory map.

The Video.MusteinGenericGPU model is bundled with SoftConsole 6.1 Renode, 
the model's source is located here:
https://github.com/AntonKrug/mustein_gpu_renode_model/
and its driver is located here:
https://github.com/AntonKrug/mustein_gpu_driver

The name "mustein" has no meaning, it is auto-generated and was chosen on purpose 
to have as little duplicates/conflicts as possible because searching for 'generic 
frame buffer peripheral' would yeild mostly irrelevant results.

The peripheral can be used to display graphical output from Renode, however 
currently there is no input support (no mouse/touch emulation).

Only hart1 is used, hart0 is put into sleep in a infinite loop, while
hart2-4 are forcefully halted  in the /renode/pse-mustein-julia.resc

Application can't use hart 0 as it requires the CPU to have floating point 
IEEE754 extension.

Project's preprocessor define **MPFS_HAL_LAST_HART=1** makes sure that HAL is 
aware that the harts2-4 are not present.

To debug/run the demo launch the **pse-mustein-julia Renode hart1 Start-platform-and-debug** launcher.

Note: The external launcher **pse-mustein-julia Renode-emulation-platform** has 
a hard-coded path to the platform location as this example is using custom 
platform bundled with the project. When renaming the project the path will break
and therefore it is necessary to keep it updated with the current project's name.

For more details read the pse-blinky's README, SoftConsole Release Notes,
Renode manual and visit:
https://www.microsemi.com/product-directory/fpga-soc/5210-mi-v-embedded-ecosystem#renode-webinar-series

