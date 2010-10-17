
enum state {
	Transition = 0,
	Authenticate = 1,
	AuthFailure = 2,
	DialPrompt = 3,
	Connected = 4,
	Disconnected = 5,
	Disconnect = 6,
	AutoDetect = 7,
	MuteAutoDetect = 8,
	RetrieveCall = 9,
	HumanDetected = 10,
	Hold = 11,
	Exit = 12
};

enum command {
	SetRegistrar = 0,
	SetStun = 1,
	SetAccount = 2,
	SetPassword = 3,
	Call = 4,
	Quit = 5,
	Disconnect = 6,
	Hold = 7,
	AutoDetect = 8,
	Retrieve = 9,
	Mute = 10,
	UnMute = 11
};
