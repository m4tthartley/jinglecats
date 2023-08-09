
#define YOU "You"
#define SANTA "Santa"

typedef struct {
	char* name;
	char* text;
} dialog_line;

typedef struct {
	dialog_line* lines;
	int count;
} dialog;

typedef struct {
	dialog santaIntro;
	dialog* current;
	int line;
	float typer;
} game_dialog;

dialog_line testLines[] = {
	{SANTA, "Ho ho hello there, Mittens!"},
	{SANTA, "Thank you for coming, I'm so glad you could make it."},
	{SANTA, "I deperately need some help around here."},
	{SANTA, "You really can't get the staff these days!"},
	{YOU,  "No problem Santa, how can I help?"},
	{SANTA, "First things first, Lewis could use some help with the choir."},
	{SANTA, "I think you're going to love what they have come up with this year."},
	{SANTA, "You can find lewis and the choir in the Studio."},
	{SANTA, "But there are also many Elves all around the Grotto, in need of help."},
	{SANTA, "If you are so inclined..."},
	{SANTA, "You could think of the Elves like side quests, Ho ho ho!"},
	{YOU, "Sounds good Santa, I'll get right on it!"},
	{SANTA, "I'm counting on you Mittens, Christmas may be saved yet!"},
};

//dialog test = {testLines, array_size(testLines)};

dialog _initDialog(dialog_line* lines, int count) {
	dialog result;
	result.lines = lines;
	result.count = count;
	return result;
}
#define initDialog(lines) _initDialog(lines, c_array_size(lines))

game_dialog initGameDialog() {
	game_dialog dialogs = {0};
	dialogs.santaIntro = initDialog(testLines);
	return dialogs;
}