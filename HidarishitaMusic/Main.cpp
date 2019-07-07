#include <Siv3D.hpp>

#define BUTTON_PLAY L"ButtonPlay"
#define BUTTON_PAUSE L"ButtonPause"
#define BUTTON_OPEN L"ButtonOpen"
#define SLIDER_POSITION L"SliderPosition"
#define SLIDER_VOLUME L"SliderVolume"

struct PlaylistItem
{
	String path;
	String title;
	double volume;
};

class SoundPlayer
{
private:

	GUI m_gui;

	Array<PlaylistItem> playlist;
	String playlistPath;
	int32 playingIndex;

	Sound m_sound;

	bool playing;

	const Font font;
	int titleOffset;

	void loadPlaylist(String path)
	{
		const auto json = JSONReader(path);

		Array<PlaylistItem> temp;
		for (const auto &jsonItem : json.root().getArray())
		{
			temp.push_back(
				{
					jsonItem[L"path"].get<String>(),
					jsonItem[L"title"].get<String>(),
					jsonItem[L"volume"].get<double>()
				});
		}

		playlist = temp;
		playlistPath = path;
		playingIndex = -1;

		setToNext();

		m_gui.button(BUTTON_PLAY).enabled = true;
	}

	void setToNext()
	{
		if (playlist.empty())
		{
			return;
		}

		playingIndex = (playingIndex + 1) % playlist.size();

		m_sound = Sound(playlist[playingIndex].path);
		setVolume(playlist[playingIndex].volume);

		titleOffset = 0;
	}

	String getPlayingTitle() const
	{
		return (playingIndex < 0) ? L"" : playlist[playingIndex].title;
	}

	int32 getTitleWidth() const
	{
		return font(getPlayingTitle()).region().w;
	}

	void setVolume(double volume)
	{
		m_sound.setVolume(volume);
		m_gui.slider(SLIDER_VOLUME).setValue(volume);
	}

public:

	SoundPlayer()
		: m_gui(GUIStyle::Default),
		font(20, Typeface::Medium, FontStyle::Outline),
		playingIndex(-1)
	{
		m_gui.add(BUTTON_PLAY, GUIButton::Create(L">"));

		m_gui.add(BUTTON_PAUSE, GUIButton::Create(L"||"));

		m_gui.addln(BUTTON_OPEN, GUIButton::Create(L"Open"));

		m_gui.addln(SLIDER_POSITION, GUISlider::Create(0.0, 1.0, 0.0, Window::Width() - 100));

		m_gui.add(SLIDER_VOLUME, GUISlider::Create(0.0, 1.0, 1.0, Window::Width() - 100));

		m_gui.setPos((Window::Width() - m_gui.getRect().w) / 2, 140);
	}

	void update()
	{
		if (playing && !m_sound.isPlaying())
		{
			setToNext();
			m_sound.play();
		}

		m_gui.button(BUTTON_PLAY).enabled = (m_sound && !m_sound.isPlaying());

		m_gui.button(BUTTON_PAUSE).enabled = m_sound.isPlaying();

		if (m_gui.button(BUTTON_PLAY).pushed)
		{
			m_sound.play();
			playing = true;
		}
		else if (m_gui.button(BUTTON_PAUSE).pushed)
		{
			m_sound.pause();
			playing = false;
		}
		else if (m_gui.button(BUTTON_OPEN).pushed)
		{
			const Array<ExtensionFilterPair> filters
			{
				ExtensionFilterPair(L"JSONファイル", L"*.json"),
				ExtensionFilterPair(L"すべてのファイル", L"*.*"),
			};
			const auto &path = Dialog::GetOpen(filters);
			if (path.has_value())
			{
				loadPlaylist(path.value());
			}
		}

		if (m_gui.slider(SLIDER_POSITION).hasChanged)
		{
			m_sound.setPosSec(m_sound.lengthSec() * m_gui.slider(SLIDER_POSITION).value);
		}

		if (m_gui.slider(SLIDER_VOLUME).hasChanged)
		{
			m_sound.setVolume(m_gui.slider(SLIDER_VOLUME).value);
		}

		m_gui.slider(SLIDER_POSITION).setValue(m_sound.streamPosSec() / m_sound.lengthSec());

		titleOffset -= 1;

		const auto w = getTitleWidth();
		if (titleOffset < -w)
		{
			titleOffset += w + Window::Width() / 2;
		}
	}

	void drawVisualizer() const
	{
		const auto& title = getPlayingTitle();
		const auto w = getTitleWidth();
		font(title).draw(22 + titleOffset, 0);
		font(title).draw(22 + titleOffset + w + Window::Width() / 2, 0);
		Rect(0, 0, 22, 40).draw(Color(0, 255, 0));
		font(L"♪").draw(2, 0);

		if (!m_sound.isPlaying())
		{
			return;
		}

		const auto fft = FFT::Analyze(m_sound);

		for (int32 i = 0; i < Window::Width(); ++i)
		{
			const double size = Pow(fft.buffer[i], 0.5) * (240 - 80);

			RectF(i, 240 / 2 - size, 1, size * 2).draw(Color(0, 0, 255));
		}
	}
};

void Main()
{
	Window::Resize(320, 300);
	Window::SetTitle(L"HidarishitaMusic");

	Graphics::SetBackground(Color(0, 255, 0));

	SoundPlayer soundPlayer;

	while (System::Update())
	{
		soundPlayer.update();

		soundPlayer.drawVisualizer();
	}
}