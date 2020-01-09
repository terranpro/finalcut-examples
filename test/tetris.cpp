#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <chrono>

#include <final/final.h>

namespace fc = finalcut;

namespace game {
struct color
{
	int r{255};
	int g{255};
	int b{255};
	int a{255};
};

struct piece
{
	int id;
	int orig_x, orig_y;
	std::vector<std::pair<int, int>> blocks;
	color c;

	virtual ~piece() {}
	virtual void rotate()
	{}
};

struct t_piece : piece
{
	t_piece()
	{
		id = 't';

		//     B       A                  C
		//   A O C  => O B  => C O A => B O
		//             C         B        A

		// xoff, yoff
		blocks.push_back({-1,  0});
		blocks.push_back({ 0, -1});
		blocks.push_back({ 1,  0});

		// color
		c = {128,0,128,255};
	}

	void rotate() override
	{
		for(auto& b : blocks) {
			auto tmp = b.second;
			b.second = b.first;
			b.first = -tmp;
		}
	}
};

struct s_piece : piece
{
	s_piece()
	{
		id = 's';

		//    A B  =>  C
		//  C O        O A
		//               B
		blocks.push_back({ 0, -1});
		blocks.push_back({ 1, -1});
		blocks.push_back({-1,  0});
	}

	void rotate() override
	{
		for(auto& b : blocks) {
			auto tmp = b.second;
			b.second = b.first;
			b.first = -tmp;
		}
	}
};

struct z_piece : piece
{
	z_piece()
	{
		id = 'z';

		//  A B           A
		//    O C   =>  O B
		//              C
		blocks.push_back({-1, -1});
		blocks.push_back({ 0, -1});
		blocks.push_back({ 1,  0});
	}

	void rotate() override
	{
		for(auto& b : blocks) {
			auto tmp = b.second;
			b.second = b.first;
			b.first = -tmp;
		}
	}
};

struct l_piece : piece
{
	std::vector<std::pair<int, int>> rots = {
		//
		{ 0,  1},
		{ 1,  0},
		{ 2,  0},
		//
		{ 0,  1},
		{ 1,  1},
		{ 0, -1},
		//
		{ 0, -1},
		{-1,  0},
		{-2,  0},
		//
		{ 0,  1},
		{ 0, -1},
		{-1, -1}
	};
	int rot_cur = 0;


	l_piece()
	{
		id = 'l';

		// O B C        C           A      C B
		// A       =>   O   =>  C B O  =>    O
		//              A B                  A
		blocks.push_back(rots[0]);
		blocks.push_back(rots[1]);
		blocks.push_back(rots[2]);
	}

	void rotate() override
	{
		rot_cur += 3;
		if ( rot_cur >= rots.size() )
			rot_cur = 0;

		blocks[0] = rots[rot_cur];
		blocks[1] = rots[rot_cur+1];
		blocks[2] = rots[rot_cur+2];
	}
};

struct j_piece : piece
{
	std::vector<std::pair<int, int>> rots = {
		//
		{ 0,  1},
		{-1,  0},
		{-2,  0},
		//
		{ 1,  0},
		{ 0,  1},
		{ 0,  2},
		//
		{ 0,  1},
		{ 1,  1},
		{ 2,  1},
		//
		{ 0, -1},
		{ 0,  1},
		{-1,  1}
	};
	int rot_cur = 0;

	j_piece()
	{
		id = 'j';

		// C B O      O A                 A
		//     A   => B     => O     =>   O
		//            C        A B C    C B
		blocks.push_back(rots[0]);
		blocks.push_back(rots[1]);
		blocks.push_back(rots[2]);
	}

	void rotate() override
	{
		rot_cur += 3;
		if ( rot_cur >= rots.size() )
			rot_cur = 0;

		blocks[0] = rots[rot_cur];
		blocks[1] = rots[rot_cur+1];
		blocks[2] = rots[rot_cur+2];
	}
};

struct o_piece : piece
{
	o_piece()
	{
		id = 'o';

		// C A
		// B O
		//
		blocks.push_back({ 0, -1});
		blocks.push_back({-1,  0});
		blocks.push_back({-1, -1});
	}
};

struct i_piece : piece
{
	i_piece()
	{
		id = 'i';

		//               O
		// O A B C  =>   A
		//               B
		//               C
		blocks.push_back({ 1,  0});
		blocks.push_back({ 2,  0});
		blocks.push_back({ 3,  0});
	}

	void rotate() override
	{
		for(auto& b : blocks) {
			int tmp = b.first;
			b.first = b.second;
			b.second = tmp;
		}
	}
};


struct engine
{
	std::size_t width{8}, height{10};
	std::vector<std::vector<int>> board;
	std::unique_ptr<piece> active_piece{};
	std::unique_ptr<piece> next_piece{generate_piece()};

	int score{0};
	int drop_height{0};

	explicit engine(std::size_t w = 8, std::size_t h = 10)
		: width(w)
		, height(h)
	{}

	void reset()
	{
		// board[y][x]
		board.resize( height, std::vector<int>(width, 0) );
	}

	std::vector<std::size_t> update()
	{
		if (!active_piece) {
			active_piece = std::move(next_piece);
			active_piece->orig_x = width / 2;
			active_piece->orig_y = 2;

			next_piece = generate_piece();

			if (check_collision())
				abort();

			cement_piece();

			return {};
		}

		clear_active_piece();

		active_piece->orig_y++;

		if (check_collision()) {
			active_piece->orig_y--;

			cement_piece();
			active_piece.reset();

			return try_clear_lines();
		} else {
			cement_piece();
		}

		return {};
	}

	void move_left()
	{
		if (!active_piece)
			return;

		if (active_piece->orig_x == 0)
			return;

		for(auto b : active_piece->blocks)
			if (active_piece->orig_x + b.first <= 0)
				return;

		clear_active_piece();

		active_piece->orig_x--;
		if (check_collision())
			active_piece->orig_x++;

		cement_piece();
	}

	void move_right()
	{
		if (!active_piece)
			return;

		if (active_piece->orig_x == width - 1)
			return;

		for(auto b : active_piece->blocks)
			if (active_piece->orig_x + b.first >= width - 1)
				return;

		clear_active_piece();

		active_piece->orig_x++;
		if (check_collision())
			active_piece->orig_x--;

		cement_piece();
	}

	void rotate()
	{
		if (!active_piece)
			return;

		clear_active_piece();

		active_piece->rotate();

		int offsets[] = {
			-1, -2, +1, +2
		};
		int orig = active_piece->orig_x;
		int i = 0;

		while (check_collision() && i < sizeof(offsets)/sizeof(offsets[0])) {
			active_piece->rotate();
			active_piece->rotate();
			active_piece->rotate();

			active_piece->orig_x = orig + offsets[i++];
			active_piece->rotate();
		}

		if (check_collision()) {
			active_piece->orig_x = orig;

			active_piece->rotate();
			active_piece->rotate();
			active_piece->rotate();
		}

		cement_piece();
	}

	void clear_active_piece()
	{
		auto x = active_piece->orig_x;
		auto y = active_piece->orig_y;

		board[y][x] = 0;

		for(auto b : active_piece->blocks)
			board[y+b.second][x+b.first] = 0;
	}

	bool check_collision() const
	{
		auto y = active_piece->orig_y;
		auto x = active_piece->orig_x;

		if (y == height || board[y][x] != 0)
			return true;

		for(auto b : active_piece->blocks)
			if (y + b.second <= 0
			    || y + b.second >= height
			    || x + b.first < 0
			    || x + b.first >= width
			    || (board[y + b.second][x + b.first] != 0
			        && board[y + b.second][x + b.first] != 'g'))
				return true;

		return false;
	}

	void cement_piece()
	{
		auto x = active_piece->orig_x;
		auto y = active_piece->orig_y;

		if (y < height && y >= 0)
			board[y][x] = active_piece->id;

		for(auto b : active_piece->blocks)
			if (y < height && y >= 0 && x >= 0 && x < width)
				board[y+b.second][x+b.first] = active_piece->id;
	}

	std::vector<std::size_t> try_clear_lines()
	{
		std::vector<std::size_t> linenumstoclear;

		for (int y = height-1; y != 0; --y) {
			bool isfull = true;

			for (int x = 0; x < width; ++x)
				if (board[y][x] == 0) {
					isfull = false;
					break;
				}

			if (isfull)
				linenumstoclear.push_back(y);
		}

		if (linenumstoclear.empty())
			return linenumstoclear;

		auto remaining = linenumstoclear;

		int cleared = 0;
		for (int beg = -1, cur = 0;
		     !remaining.empty() && cleared < 4;
		     ++beg,++cur,++cleared)
		{
			board.erase(board.begin() + remaining[0],
			            board.begin() + remaining[0] + 1);

			remaining.erase(remaining.begin());
		}

		std::reverse(board.begin(), board.end());
		while(board.size() != height)
			board.push_back( std::vector<int>(width, 0) );
		std::reverse(board.begin(), board.end());

		switch(cleared) {
		case 1:
			score += 40;
			break;
		case 2:
			score += 100;
			break;
		case 3:
			score += 300;
			break;
		case 4:
		default:
			score += 1200;
			break;
		}

		return linenumstoclear;
	}

	std::unique_ptr<piece> generate_piece() const {
		static int next_id = 1;

		switch(next_id++ % 7) {
		case 0:
			return std::make_unique<t_piece>();

		case 1:
			return std::make_unique<s_piece>();

		case 2:
			return std::make_unique<z_piece>();

		case 3:
			return std::make_unique<o_piece>();

		case 4:
			return std::make_unique<l_piece>();

		case 5:
			return std::make_unique<j_piece>();

		case 6:
		default:
			return std::make_unique<i_piece>();
		}
	}

	std::unique_ptr<piece> ghost_piece() {
		if (!active_piece)
			return nullptr;

		auto gp = std::make_unique<piece>();
		gp->blocks = active_piece->blocks;
		gp->id = active_piece->id;
		gp->c = active_piece->c;
		gp->orig_x = active_piece->orig_x;
		gp->orig_y = active_piece->orig_y;

		clear_active_piece();

		while( !check_collision() ) {
			active_piece->orig_y++;
		}

		active_piece->orig_y--;
		int tmp = active_piece->orig_y;

		active_piece->orig_y = gp->orig_y;
		gp->orig_y = tmp;

		cement_piece();

		return gp;
	}

	std::ostream& print(std::ostream& os) const
	{
		for(int y = 0; y < board.size(); ++y) {

			for(int x = 0; x < board[0].size(); ++x)
				if (board[y][x] == 0)
					std::cout << "0" << " ";
				else os << (char)board[y][x] << " ";

			os << "\n";
		}

		return os;
	}

	friend std::ostream& operator<<(std::ostream& os, engine const& eng) {
		return eng.print(os);
	}
};

}

class TetrisWindow : public fc::FWindow
{
public:
	int startx{1}, starty{1};
	int update_ms = 300;
	int timer_id{0};

	game::engine engine{15, 19};

	std::size_t scale_x = 4, scale_y = 2;
	std::size_t win_width = 32, win_height = 21;

	bool draw_ghost = true;

	bool animating = false;
	std::chrono::high_resolution_clock::time_point animating_start{};
	std::size_t animating_frame = 0;
	std::vector<std::size_t> cleared_lines;
	decltype(game::engine::board) animating_board;


public:
	explicit TetrisWindow(fc::FWidget& parent)
		: fc::FWindow(&parent)
	{
		resizeWindow();

		engine.reset();

		timer_id = addTimer(update_ms);
	}

	void onKeyPress (fc::FKeyEvent* ev) override
	{
		if (animating)
			return;

		auto key = ev->key();
		switch(key) {
		case fc::fc::Fkey_escape:
		case 'q':
			close();
			break;

		case fc::fc::Fkey_left:
		case 'a':
			engine.move_left();
			break;

		case fc::fc::Fkey_right:
		case 'd':
			engine.move_right();
			break;

		case fc::fc::Fkey_up:
		case 'r':
			engine.rotate();
			break;

		case 'g':
			draw_ghost = !draw_ghost;
			break;

		case fc::fc::Fkey_down:
			delTimer(timer_id);
			if (!doUpdate())
				timer_id = addTimer(update_ms);
			break;

		case fc::fc::Fkey_space:
			while(engine.active_piece)
				doUpdate();
			break;

		case 'x':
			scale_x = std::max(scale_x-1, (std::size_t)1);
			resizeWindow();
			break;
		case 'X':
			scale_x = scale_x + 1;
			resizeWindow();
			break;

		case 'y':
			scale_y = std::max(scale_y-1, (std::size_t)1);
			resizeWindow();
			break;
		case 'Y':
			scale_y = scale_y + 1;
			resizeWindow();
			break;

		default:
			fc::FWindow::onKeyPress(ev);
		}

		redraw();
	}

	void resizeWindow()
	{
		getRootWidget()->clearArea( );

		setGeometry({3, 3, scale_x*win_width, win_height*scale_y});
	}

	fc::fc::colornames getPieceColor(char p) const
	{
		std::vector<fc::fc::colornames> animating_colors = {
			fc::fc::Grey100,
			fc::fc::Grey93,
			fc::fc::Grey89,
			fc::fc::Grey85,
			fc::fc::Grey84,
			// fc::fc::Grey82,
			// fc::fc::Grey78,
			// fc::fc::Grey74,
			// fc::fc::Grey70
		};

		switch(p) {
		case 't':
			return fc::fc::Purple;

		case 's':
			return fc::fc::Red;

		case 'z':
			return fc::fc::Blue;

		case 'l':
			return fc::fc::Orange1;

		case 'j':
			return fc::fc::DarkSeaGreen1;

		case 'o':
			return fc::fc::Yellow;

		case 'i':
			return fc::fc::Cyan;

		case '9':
			return fc::fc::DarkRed2;
		case '8':
			return fc::fc::DarkRed;
		case '7':
			return fc::fc::Red3;
		case '6':
			return fc::fc::Red2;
		case '5':
			return fc::fc::Red1;
		case '4':
			return fc::fc::Red;
		case '3':
			return fc::fc::MediumVioletRed;
		case '2':
			return fc::fc::LightRed;
		case '1':
			return fc::fc::PaleVioletRed1;
		case '0':
			return fc::fc::Black;

		case 0:
		default:
			if (animating)
				return animating_colors[ rand() % animating_colors.size() ];
			return fc::fc::Black;
		}
	}

	bool doUpdate()
	{
		animating_board = engine.board;

		cleared_lines = engine.update();
		if (cleared_lines.size() > 0)
			startAnimation();

		return cleared_lines.size();
	}

	void draw() override
	{
		// clearArea(getVirtualDesktop(), fc::fc::Red2);
		setColor(fc::fc::LightBlue, fc::fc::Cyan);

		clearArea( fc::fc::MediumShade );

		print() << fc::FPoint(startx,starty) << "well well well "
		        << animating_frame << fc::fc::FullBlock;

		for(int y = 0; y != engine.board.size(); ++y) {
			for(int x = 0; x < engine.board[y].size(); ++x) {
				fc::fc::colornames color;
				if (animating)
					color = getPieceColor(animating_board[y][x]);
				else
					color = getPieceColor(engine.board[y][x]);
				setColor(color, color);

				for(int sy = 0; sy < scale_y; ++sy) {
					for(int sx = 0; sx < scale_x; ++sx)
						print() << fc::FPoint(x*scale_x + 1 + sx, y*scale_y + 1 + sy) << " ";
				}

			}
		}

		drawGhostPiece();
		drawActivePiece();

		drawScore();

		drawNextPiece();

		setColor(fc::fc::White, fc::fc::Grey0);
		drawBorder();
	}

	void drawScore()
	{
		setColor(fc::fc::White, fc::fc::Black);

		for(int y = 1; y < 10; ++y) {
			for(int x = 18; x < 32; ++x) {
				for(int sy = 0; sy < scale_y; ++sy)
					for(int sx = 0; sx < scale_x; ++sx)
						print() << fc::FPoint(x*scale_x + sx, y*scale_y + sy) << " ";
			}
		}

		print() << fc::FPoint(20*scale_x, 3*scale_y) << "Score: " << engine.score;
	}

	void drawNextPiece()
	{
		int startx = 25;
		int starty = 8;
		auto color = getPieceColor(engine.next_piece->id);

		setColor(fc::fc::White, fc::fc::Black);
		print() << fc::FPoint(20*scale_x, 6*scale_y) << "Next: ";

		setColor(color, color);
		for(int sy = 0; sy < scale_y; ++sy)
			for(int sx = 0; sx < scale_x; ++sx)
				print() << fc::FPoint(startx*scale_x + sx, starty*scale_y + sy) << " ";

		for(auto b : engine.next_piece->blocks)
			for(int sy = 0; sy < scale_y; ++sy)
				for(int sx = 0; sx < scale_x; ++sx)
					print() << fc::FPoint((startx+b.first)*scale_x + sx,
					                      (starty+b.second)*scale_y + sy)
					        << " ";
	}

	void drawGhostPiece()
	{
		if (!draw_ghost)
			return;

		auto gp = engine.ghost_piece();
		if (!gp)
			return;

		int x = gp->orig_x, y = gp->orig_y;

		setColor(fc::fc::Grey30, fc::fc::Grey30);

		gp->blocks.push_back({0, 0});

		for(auto b : gp->blocks) {
			for(int sy = 0; sy < scale_y; ++sy)
				for(int sx = 0; sx < scale_x; ++sx)
					print() << fc::FPoint((x+b.first)*scale_x + 1 + sx, (y+b.second)*scale_y + 1 + sy) << " ";
		}
	}

	void drawActivePiece()
	{
		auto& ap = engine.active_piece;
		if (!ap)
			return;

		int x = ap->orig_x, y = ap->orig_y;

		auto color = getPieceColor(ap->id);

		setColor(color, color);

		for(int sy = 0; sy < scale_y; ++sy)
			for(int sx = 0; sx < scale_x; ++sx)
				print() << fc::FPoint((x)*scale_x + 1 + sx, (y)*scale_y + 1 + sy) << " ";

		for(auto b : ap->blocks) {
			for(int sy = 0; sy < scale_y; ++sy)
				for(int sx = 0; sx < scale_x; ++sx)
					print() << fc::FPoint((x+b.first)*scale_x + 1 + sx, (y+b.second)*scale_y + 1 + sy) << " ";
		}
	}

	void onTimer(fc::FTimerEvent *) override
	{
		if (animating)
			onAnimationTimer(nullptr);
		else
			onEngineTimer(nullptr);
	}

	void onEngineTimer(fc::FTimerEvent*)
	{
		startx++; if (startx >= getWidth()) { startx = 0; ++starty; }
		starty++; if (starty >= getHeight()) { starty = 0; startx = 0; }

		doUpdate();

		redraw();
	}

	void startAnimation()
	{
		std::size_t animation_ms = 1;

		animating = true;
		animating_start = std::chrono::high_resolution_clock::now();
		animating_frame = 0;

		delTimer(timer_id);
		timer_id = addTimer(animation_ms);

		for (int y = 0; y < cleared_lines.size(); ++y) {
			for(int x = 0; x < engine.width; ++x) {
				animating_board[cleared_lines[y]][x] = '9';
			}
		}
	}

	void onAnimationTimer(fc::FTimerEvent*)
	{
		std::size_t animation_time_ms = 250;

		++animating_frame;

		auto dur = std::chrono::high_resolution_clock::now() - animating_start;
		auto dur_ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur);
		if ( dur_ms.count() > animation_time_ms ) {
			animating = false;
			delTimer(timer_id);
			timer_id = addTimer(update_ms);
			cleared_lines.clear();
		}


		// X X X X X X
		//     ^ ^
		//     l r
		// X X X X X X X
		//     l 0 r
		bool iseven = engine.width % 2 == 0;

		for (int i = 0; i < animating_board.size(); ++i) {
			if (!iseven && animating_board[i][engine.width / 2] == '9')
				animating_board[i][engine.width / 2] = '0';
			else if (!iseven && animating_board[i][engine.width / 2] == '0')
				animating_board[i][engine.width / 2] = '9';

			for (int left = engine.width / 2 - 1,
				     right = iseven ? engine.width / 2 : engine.width / 2 + 1;
			     left >= 0 && right < engine.width;
			     --left,++right)
			{
				if (animating_board[i][left] == '9')
					animating_board[i][left] = '0';
				else if (animating_board[i][left] == '0')
					animating_board[i][left] = '9';

				if (animating_board[i][right] == '9')
					animating_board[i][right] = '0';
				else if (animating_board[i][right] == '0')
					animating_board[i][right] = '9';
			}
		}

		redraw();
	}

};

int main(int argc, char **argv)
{
#if 1
	fc::FApplication app{argc, argv};
	TetrisWindow mainwindow{app};

	mainwindow.engine.board = {
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
		{ 's','s','s','s','s','s','s', 0,'s','s','s','s','s','s','s' },
		{ 's','s','s','s','s','s','s', 0,'s','s','s','s','s','s','s' },
		{ 's','s','s','s','s','s','s', 0,'s','s','s','s','s','s','s' },
		{ 's','s','s','s','s','s','s', 0,'s','s','s','s','s','s','s' },
	};

	app.setMainWidget(&mainwindow);

	mainwindow.show();

	return app.exec();
#else
	std::vector<std::vector<int>> tc1 = {
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},

		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{'9','9','9','9','9','9','9','9'},
	};

	std::vector<std::vector<int>> tc2 = {
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},

		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{'9','9','9','9','9','9','9','9'},
		{'9','9','9','9','9','9','9','9'},
	};

	std::vector<std::vector<int>> tc3 = {
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},

		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{'9','9','9','9','9','9','9','9'},
		{0,0,0,0,0,0,0,0},
		{'9','9','9','9','9','9','9','9'},
	};

	std::vector<std::vector<int>> tc4 = {
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},

		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{'9','9','9', 0 , 0 ,'9','9','9'},
		{'9','9','9','9','9','9','9','9'},
		{'9','9','9','9','9','9','9','9'},
		{'9','9','9','9','9','9','9','9'},
	};

	std::vector<std::vector<int>> tc5 = {
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},

		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{'9','9','9', 0 , 0 ,'9','9','9'},
		{'9','9','9','9','9','9','9','9'},
		{'9','9', 0 , 0 , 0 ,'9','9','9'},
		{'9','9','9','9','9','9','9','9'},
	};

	std::vector<std::vector<int>> tc6 = {
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},

		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0},
		{'9','9','9','9', 0 ,'9','9','9'},
		{'9','9','9', 0 , 0 ,'9','9','9'},
		{'9','9','9','9', 0 ,'9','9','9'},
		{'9','9', 0 , 0 , 0 ,'9','9','9'},
		{'9','9','9', 0 ,'9','9','9','9'},
	};

	game::engine eng{8, 20};
	eng.reset();

	eng.board = tc6;

	std::cout << eng << "\n";

	std::string input;
	while(std::getline(std::cin, input)) {
		if (input == "a")
			eng.move_left();
		else if (input == "d")
			eng.move_right();
		else if (input == "r")
			eng.rotate();

		eng.update();
		eng.print(std::cout);
		std::cout << "\n\n";
	}

	return 0;
#endif
}
