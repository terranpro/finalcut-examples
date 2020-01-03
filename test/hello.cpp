#include <iostream>

#include <final/final.h>

namespace fc = finalcut;

class HelloDialog : public fc::FDialog
{
	fc::FLabel labelDim{this};
	fc::FLabel labelPos{this};

public:
	HelloDialog(fc::FWidget& widget)
		: FDialog(&widget)
	{
		setText("Top Kek - Hello Finalcut");
		setPos(fc::FPoint(10, 1));
		setSize(fc::FSize(50,25));

		setMinimumSize({25, 1});
		setMaximumSize({100,35});

		setResizeable();

		labelDim.addCallback("mouse-release",
		                     [&](fc::FWidget *w, void *) {
			                     setSize(fc::FSize(25, 23));
		                     });

		labelDim.setPos({3,2});
		labelDim.setMaximumSize({55, 1});
		labelDim.setMinimumSize({5, 1});
		labelDim.setSize({30, 1});

		//labelDim.setGeometry({3, 2}, {30, 1});
		labelPos.setGeometry({3, 3}, {30, 1});

		updateLabel();

		addTimer(100);
	}

	void updateLabel()
	{
		fc::FString l{""};

		auto width = getClientWidth(); // labelDim.getWidth();
		auto height = getClientHeight(); // labelDim.getHeight();

		l.sprintf("Dimensions: %d x %d", width, height);

		labelDim.setText(l);

		auto x = getX();
		auto y = getY();

		l.clear();
		l.sprintf("Dialog Position: %d , %d", x, y);

		labelPos.setText(l);
	}

	void onKeyPress(fc::FKeyEvent *ev) override
	{
		auto key = ev->key();
		switch(key) {
		case fc::fc::Fkey_escape:
		case 'q':
			close();
			break;

		case 'r':
			redraw();
			break;

		default:
			fc::FDialog::onKeyPress(ev);
			break;

		}
	}

	void adjustSize() override
	{
		fc::FDialog::adjustSize();

		labelDim.setSize({getClientWidth(), getClientHeight()});

		//updateLabel();
	}

	void onTimer (fc::FTimerEvent *ev) override
	{
		labelDim << ".";

		redraw();
	}

	void setChildText(fc::FString const& str)
	{
		labelDim.setText(str);
	}
};

class HelloApplication : public fc::FApplication
{
	HelloDialog hello;

public:
	HelloApplication(int& argc, char **argv)
		: fc::FApplication(argc, argv)
		, hello(*this)
	{
		// setFWidgetColors().dialog_bg = fc::fc::colornames::Cornsilk1;
		// setFWidgetColors().dialog_fg = fc::fc::colornames::DeepPink5;
		hello.setColor(fc::fc::colornames::Brown,
		               fc::fc::colornames::DarkCyan2);

		setMainWidget(&hello);

		hello.show();
	}

	void onResize(fc::FResizeEvent *ev) override
	{
		hello.setChildText("for the glory of terran");

		fc::FApplication::onResize(ev);
	}
};


int main(int argc, char **argv)
{
	HelloApplication app{argc, argv};

	return app.exec();
}
