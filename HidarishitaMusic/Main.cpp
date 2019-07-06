# include <Siv3D.hpp>

class SoundPlayer
{
private:

	GUI m_gui;

	Sound m_sound;

	const Font font;
	String title;
	int titleOffset;

public:

	SoundPlayer()
		: m_gui(GUIStyle::Default),
		font(20, Typeface::Medium, FontStyle::Outline)
	{
		m_gui.add(L"PlayButton", GUIButton::Create(L">"));

		m_gui.add(L"PauseButton", GUIButton::Create(L"||"));

		m_gui.addln(L"OpenButton", GUIButton::Create(L"Open"));

		m_gui.add(L"Slider", GUISlider::Create(0.0, 1.0, 0.0, Window::Width() - 100));

		m_gui.setPos((Window::Width() - m_gui.getRect().w) / 2, Window::Height() - 100);
	}

	void update()
	{
		m_gui.button(L"PlayButton").enabled = (m_sound && !m_sound.isPlaying());

		m_gui.button(L"PauseButton").enabled = m_sound.isPlaying();

		if (m_gui.button(L"PlayButton").pushed)
		{
			m_sound.play();
		}
		else if (m_gui.button(L"PauseButton").pushed)
		{
			m_sound.pause();
		}
		else if (m_gui.button(L"OpenButton").pushed)
		{
			m_sound.pause();

			m_sound = Dialog::OpenSound();

			title = L"曲タイトル";
			titleOffset = 0;
		}

		if (m_gui.slider(L"Slider").hasChanged)
		{
			m_sound.setPosSec(m_sound.lengthSec() * m_gui.slider(L"Slider").value);
		}

		m_gui.slider(L"Slider").setValue(m_sound.streamPosSec() / m_sound.lengthSec());

		titleOffset -= 1;

		const auto region = font(title).region();
		if (titleOffset < -region.w) {
			titleOffset += region.w + Window::Width() / 2;
		}
	}

	void drawVisualizer() const
	{
		const auto region = font(title).region();
		font(title).draw(22 + titleOffset, 0);
		font(title).draw(22 + titleOffset + region.w + Window::Width() / 2, 0);
		Rect(0, 0, 22, 40).draw(Color(0, 255, 0));
		font(L"♪").draw(2, 0);

		if (!m_sound.isPlaying())
		{
			return;
		}

		const auto fft = FFT::Analyze(m_sound);

		for (int32 i = 0; i < Window::Width(); ++i)
		{
			const double size = Pow(fft.buffer[i], 0.6) * Window::Height();

			RectF(i, Window::Height() / 2 + size, 1, -size * 2).draw(Color(0, 0, 255));
		}
	}
};

void Main()
{
	Window::Resize(320, 240);
	Window::SetTitle(L"HidarishitaMusic");

	Graphics::SetBackground(Color(0, 255, 0));

	SoundPlayer soundPlayer;

	while (System::Update())
	{
		soundPlayer.update();

		soundPlayer.drawVisualizer();
	}
}