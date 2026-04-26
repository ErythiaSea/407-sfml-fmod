#include "Animation.h"

void Animation::addFrame(sf::IntRect frameRect, float frameSpeed)
{
	frames.push_back({ frameRect, frameSpeed });
}

void Animation::addFramesRow(sf::IntRect frameRect, int numFrames, int offset)
{
	frames.push_back({ frameRect, 1.0f });

	sf::IntRect rect = frameRect;
	for (int i = 1; i < numFrames; i++) {
		rect = { {rect.position.x + rect.size.x + offset, rect.position.y}, rect.size };
		frames.push_back({ rect, 1.0f });
	}
}

sf::IntRect Animation::getCurrentFrame()
{
	if (!h_flip && !v_flip)
		return frames[current_frame].rect;

	sf::IntRect r = frames[current_frame].rect;
	if (h_flip && v_flip) {
		return sf::IntRect{ {r.position.x + r.size.x, r.position.y + r.size.y}, {-(r.size.x), -(r.size.y)}};
	}
	if (h_flip) {
		return sf::IntRect{ {r.position.x + r.size.x, r.position.y}, {-(r.size.x), r.size.y} };
	}
	// v_flip case
	return sf::IntRect{ {r.position.x, r.position.y + r.size.y}, {r.size.x, -(r.size.y)} };
}

void Animation::setFrameIndex(int frameIndex)
{
	if (frameIndex == current_frame) return;
	current_frame = frameIndex;
	frame_changed = true;
}

bool Animation::hasFrameChanged()
{
	if (frame_changed) {
		frame_changed = false;
		return true;
	}
	return false;
}

void Animation::setFrameSpeed(int frameIndex, float speed)
{
	frames[frameIndex].speed = speed;
}

float Animation::getFrameSpeed(int frameIndex)
{
	return frames[frameIndex].speed;
}

void Animation::setHorizontalFlip(bool flip)
{
	h_flip = flip;
	frame_changed = true;
}

void Animation::setVerticalFlip(bool flip)
{
	v_flip = flip;
	frame_changed = true;
}

void Animation::reset()
{
	current_frame = 0;
	elapsed_time = 0.0f;
	frame_changed = true;
}

void Animation::animate(float dt)
{
	if (!playing) return;

	elapsed_time += (dt * getFrameSpeed(current_frame));
	if (elapsed_time <= animation_speed) return;
	elapsed_time -= animation_speed;

	int frameCount = getFrameCount();

	if (playing_forward) {
		if (current_frame + 1 < frameCount) {
			current_frame++;
			frame_changed = true;
			return;
		}
	}
	else if (current_frame - 1 >= 0) {
		current_frame--;
		frame_changed = true;
		return;
	}

	if (loop_type == AnimLoopType::NoLoop) {
		playing = false;
		return;
	}

	frame_changed = true;
	if (loop_type == AnimLoopType::Loop) {
		if (playing_forward) {
			current_frame = 0;
			return;
		}
		else {
			current_frame = frameCount - 1;
			return;
		}
	}

	// boomerang case
	playing_forward = !playing_forward;
	if (playing_forward) {
		current_frame = current_frame + 1;
	}
	else {
		current_frame = current_frame - 1;
	}
	return;
}
