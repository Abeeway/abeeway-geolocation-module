
	aos-app-demo: A sample application based on the Abeeway module SDK (AOS).

	This folder contains a sample project for the Abeeway module that can be
	imported in the STM32CubeIDE development  environment. It is intended to
	be used as an example on using the AOS SDK and a number of AOS services.

1. Installing the project files and setting up the STM32CubeIDE project.

	Prerequisites are a working installation of the STM32CubeIDE development
	environment.  This Eclipse based development environment can be obtained
	free of charge from ST's web site, https://www.st.com/ .

	To install the demo project, follow the four simple steps below:

	1. Create the directory where you want to base your development, here 'module-dev'.

		marc@o ~ $ mkdir module-dev
		marc@o ~ $ cd module-dev/

	2. Extract the contents of the application tarball to that directory.

		marc@o ~/module-dev $ tar xzf ~/aos-app-demo-2022.07.05.tgz

	3. Extract the contents of the sdk tarball to the application directory, 'aos-app-demo'.

		marc@o ~/module-dev $ cd aos-app-demo/
		marc@o ~/module-dev/aos-app-demo $ tar xzf ~/aos-sdk-2022.07.05-Release.tgz

	4. Import the project into the STM32CubeIDE, using 'module-dev' as the workspace.
	   This can be done from the command line if you know where your copy of STM32CubeIDE
	   has been installed, here /opt/st/stm32cubeide_1.8.0:

		marc@o ~/module-dev/aos-app-demo $ cd ..
		marc@o ~/module-dev $ /opt/st/stm32cubeide_1.8.0/stm32cubeide -data . -import ./aos-app-demo/.project

	   Alternatively, you can import the project from the IDE by right clicking
	   in the IDE Project Explorer window, selecting 'Import...', then 'General',
	   and finally 'Existing Projects into workspace').

	From here you can modify, build, flash and debug the project, all from the STM32CubeIDE.

2. Using the demo

	The demo application is a low power FreeRTOS application, based on the AOS SDK.

	It is controlled through a command line interface, accessible through the USB-serial
	controller available on the integrated ST-Link on the Abeeway Evaluation Kit.

	The serial port must be set to 57600 bauds, 8 data bits, no parity, one stop bit (8N1).

	Log in with the pin code '123' or '456'. The former provides normal user access, the
	latter provides super-user access, making different sets of CLI commands available.

	The 'help' command is available throughout, but typing any invalid command or
	omitting a command option will usually show the available commands or options.

		super> help
		ble       <cmd> BLE commands
		gnss      <cmd> GNSS commands (MT3333)
		gpio      <cmd> GPIO related commands
		help      <cmd> Display help information
		logout    Disconnect the CLI
		lora      <cmd> LoRa related commands
		pwm       <cmd> PWM related commands
		?         Display all helps
		settings  <cmd> LoRa settings related commands
		system    <cmd> System commands
		uart      <cmd> UART commands
		wifi      <cmd> WIFI related commands
		super>

	Commands may be abbreviated as long as they remain unambiguous.

		super> wi
		Missing options for wifi
		Options are:
				scan      : Perform a WIFI scan
				info      : Display WIFI information
				results   : Show WIFI scan results
				parameters: parameter related commands
		ERROR
		super> wi sc
		Scan start
		Number of results: 1
		 MAC Address: 0x00 0x0c 0x43 0x26 0x60 0x60, RSSI: -56
		OK
		super>

	Feel free to experiment with the different commands, and refer to the source
	code to understand how to perform specific operations.
