#pragma once
#include "SFML/Graphics.hpp"

struct Frame {
	sf::IntRect rect;
	float speed = 1.0f;
};

enum class AnimLoopType {
	NoLoop = 0,
	Loop = 1,
	Boomerang = 2
};

class Animation
{
public:
	Animation() {};
	~Animation() {};

	// Add a frame to the animation. frameSpeed is a speed modifier for this frame (see setFrameSpeed).
	void addFrame(sf::IntRect frameRect, float frameSpeed = 1.0f);
	// Add a batch of frames to the animation. Offset is the gap in pixels between each frame.
	void addFramesRow(sf::IntRect frameRect, int numFrames, int offset = 0);

	// Get the IntRect for the current frame of animation.
	sf::IntRect getCurrentFrame();

	// Return the index of the current frame in the vector.
	int getFrameIndex() const { return current_frame; }
	// Return the number of frames in the vector.
	int getFrameCount() { return frames.size(); };
	// Jump to a specific frame in the animation.
	void setFrameIndex(int frameIndex);

	// Returns true if the frame has changed since the last time this function was called.
	// Includes flipping and resetting.
	bool hasFrameChanged();

	// Set the speed of animation using the time in seconds between frames.
	void setAnimationSpeed(float frameTime) { animation_speed = frameTime; };
	// Set the speed of animation using the rate of frames per second.
	void setAnimationFps(float fps) { animation_speed = 1.0f / fps; };
	// Get the time in seconds between animation frames.
	float getAnimationSpeed() const { return animation_speed; }

	// Set the speed modifier of one specific frame, relative to the animation framerate. 1 is default.
	// e.g. 0.5 makes a frame persist twice as long, 2.0 makes it persist half as long.
	void setFrameSpeed(int frameIndex, float speed);
	// Get the speed % of one specific frame.
	float getFrameSpeed(int frameIndex);

	// Set the animation loop type. NoLoop disables looping, Loop enables looping,
	// Boomerang reverses the play direction at the start/end
	void setLooping(AnimLoopType loopType) { loop_type = loopType; }
	// Get the animation loop type.
	AnimLoopType getLooping() const { return loop_type; };

	// Set the play direction of the animation.
	void setPlayDirection(bool forward) { playing_forward = forward; }
	// Get the direction of play.
	bool isPlayingForward() const { return playing_forward; }

	// Set the animation's horizontal flip.
	void setHorizontalFlip(bool flip);
	// Set the animation's vertical flip.
	void setVerticalFlip(bool flip);
	// Get the animation's horizontal flip.
	bool getHorizontalFlip() const { return h_flip; }
	// Get the animation's vertical flip.
	bool getVerticalFlip() const { return v_flip; }

	// Play the animation.
	void play() { playing = true; }
	// Pause the animation.
	void pause() { playing = false; }
	// Reset the animation.
	void reset();
	// Get whether or not this animation is being played.
	bool isPlaying() const { return playing; }

	// Advance the animation by the time elapsed. Returns true if the frame has changed.
	void animate(float dt);

private:
	std::vector<Frame> frames;
	int current_frame = 0;
	float elapsed_time = 0;

	AnimLoopType loop_type = AnimLoopType::Loop;
	float animation_speed = 1.0f;
	bool playing = true;
	bool playing_forward = true;

	bool frame_changed = false;

	bool h_flip = false;
	bool v_flip = false;
};

