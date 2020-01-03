#include <iostream>

#include <final/final.h>

namespace fc = finalcut;

class HelloDialog : public fc::FDialog
{
	fc::FLabel labelDim{this};

public:
	HelloDialog(fc::FWidget& widget)
		: FDialog(&widget)
	{
		setText("Hello Finalcut - Size Issue");
		setPos(fc::FPoint(10, 1));
		setSize(fc::FSize(50,25));

		setMinimumSize({15, 4});
		setMaximumSize({100, 35});

		setResizeable(true);

		labelDim.setPos({3,2});
		labelDim.setMinimumSize({25, 1});

		labelDim.setSize({30,1});

		labelDim = "Size: ";
		labelDim << labelDim.getWidth() << "x" << labelDim.getHeight();

		addTimer(100);
	}

	void onTimer (fc::FTimerEvent *ev) override
	{
		labelDim.getText() << ".";

		// adjustSize();

		redraw();
	}

	void adjustSize() override
	{
		labelDim = "Size: ";
		labelDim << labelDim.getWidth() << "x" << labelDim.getHeight();

		fc::FDialog::adjustSize();
	}
};

int main(int argc, char **argv)
{
	fc::FApplication app{argc, argv};
	HelloDialog dialog{app};
	app.setMainWidget(&dialog);

	dialog.show();

	return app.exec();
}
