// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the DJV project.

#include <djv/App/FrameActions.h>

#include <djv/App/App.h>
#include <djv/App/MainWindow.h>
#include <djv/UI/Viewport.h>

#include <algorithm>
#include <cmath>

namespace djv
{
    namespace app
    {
        namespace
        {
            //! A second of frames at the playback speed, back or forward,
            //! kept in the in/out range.
            void stepSecond(const std::shared_ptr<tl::Player>& player, int direction)
            {
                const OTIO_NS::TimeRange& range = player->getInOutRange();
                const double rate = range.duration().rate();
                const double frames = std::max(1.0, std::round(player->getSpeed()));
                if (rate <= 0.0)
                    return;
                const OTIO_NS::RationalTime t =
                    player->getCurrentTime().rescaled_to(rate) +
                    OTIO_NS::RationalTime(direction * frames, rate);
                player->seek(std::clamp(
                    t,
                    range.start_time(),
                    range.end_time_inclusive()));
            }
        }

        namespace
        {
            //! The places Up and Down go between, as Blender's go between
            //! keyframes: the start, the last frame written of each part of
            //! the sequence rendered again, and the end.
            std::vector<OTIO_NS::RationalTime> getRefreshStops(
                const std::shared_ptr<App>& app,
                const std::shared_ptr<tl::Player>& player)
            {
                const OTIO_NS::TimeRange& range = player->getInOutRange();
                const double rate = range.duration().rate();
                std::vector<OTIO_NS::RationalTime> out;
                out.push_back(range.start_time());
                for (const auto& mark : app->getRefreshMarks())
                {
                    const OTIO_NS::RationalTime t =
                        mark.rescaled_to(rate) - OTIO_NS::RationalTime(1.0, rate);
                    if (t > range.start_time() && t < range.end_time_inclusive())
                    {
                        out.push_back(t);
                    }
                }
                out.push_back(range.end_time_inclusive());
                return out;
            }

            void seekRefresh(const std::shared_ptr<App>& app, int direction)
            {
                auto player = app->observePlayer()->get();
                if (!player)
                    return;
                const auto stops = getRefreshStops(app, player);
                const double rate = player->getInOutRange().duration().rate();
                const OTIO_NS::RationalTime current = player->getCurrentTime().rescaled_to(rate).round();
                if (direction > 0)
                {
                    for (const auto& stop : stops)
                    {
                        if (stop.round() > current)
                        {
                            player->seek(stop);
                            return;
                        }
                    }
                }
                else
                {
                    for (auto i = stops.rbegin(); i != stops.rend(); ++i)
                    {
                        if (i->round() < current)
                        {
                            player->seek(*i);
                            return;
                        }
                    }
                }
            }
        }

        struct FrameActions::Private
        {

            std::shared_ptr<ftk::Observer<std::shared_ptr<tl::Player> > > playerObserver;
        };

        void FrameActions::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow)
        {
            IActions::_init(context, app, "Frame");
            FTK_P();

            // Register the commands.
            auto appWeak = std::weak_ptr<App>(app);
            _addCommand(
                "Start",
                "Go to the start frame.",
                [appWeak](const nlohmann::json&)
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->gotoStart();
                        }
                    }
                });

            _addCommand(
                "End",
                "Go to the end frame.",
                [appWeak](const nlohmann::json&)
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->gotoEnd();
                        }
                    }
                });

            _addCommand(
                "Prev",
                "Go to the previous frame.",
                [appWeak](const nlohmann::json&)
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->framePrev();
                        }
                    }
                });

            _addCommand(
                "PrevX10",
                "Go to the previous frame X10.",
                [appWeak](const nlohmann::json&)
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->timeAction(tl::TimeAction::FramePrevX10);
                        }
                    }
                });

            _addCommand(
                "PrevX100",
                "Go back a second.",
                [appWeak](const nlohmann::json&)
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            stepSecond(player, -1);
                        }
                    }
                });

            _addCommand(
                "Next",
                "Go to the next frame.",
                [appWeak](const nlohmann::json&)
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->frameNext();
                        }
                    }
                });

            _addCommand(
                "NextX10",
                "Go to the next frame X10.",
                [appWeak](const nlohmann::json&)
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            player->timeAction(tl::TimeAction::FrameNextX10);
                        }
                    }
                });

            _addCommand(
                "NextX100",
                "Go forward a second.",
                [appWeak](const nlohmann::json&)
                {
                    if (auto app = appWeak.lock())
                    {
                        if (auto player = app->observePlayer()->get())
                        {
                            stepSecond(player, 1);
                        }
                    }
                });

            auto mainWindowWeak = std::weak_ptr<MainWindow>(mainWindow);
            _addCommand(
                "FocusCurrent",
                "Set the keyboard focus to the current frame editor.",
                [mainWindowWeak](const nlohmann::json&)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->focusCurrentFrame();
                    }
                });

            _addCommand(
                "NextRefresh",
                "Go to where the next part of the sequence rendered again has got, or to the end.",
                [appWeak](const nlohmann::json&)
                {
                    if (auto app = appWeak.lock())
                    {
                        seekRefresh(app, 1);
                    }
                });
            _addCommand(
                "PrevRefresh",
                "Go to where the previous part of the sequence rendered again has got, or to the start.",
                [appWeak](const nlohmann::json&)
                {
                    if (auto app = appWeak.lock())
                    {
                        seekRefresh(app, -1);
                    }
                });

            // Create the actions.
            _actions["Start"] = ftk::Action::create(
                "Go To Start",
                "FrameStart",
                _command("Start"));
            _actions["End"] = ftk::Action::create(
                "Go To End",
                "FrameEnd",
                _command("End"));
            _actions["Prev"] = ftk::Action::create(
                "Previous Frame",
                "FramePrev",
                _command("Prev"));
            _actions["PrevX10"] = ftk::Action::create(
                "Previous Frame X10",
                _command("PrevX10"));
            _actions["PrevX100"] = ftk::Action::create(
                "Back a Second",
                _command("PrevX100"));
            _actions["Next"] = ftk::Action::create(
                "Next Frame",
                "FrameNext",
                _command("Next"));
            _actions["NextX10"] = ftk::Action::create(
                "Next Frame X10",
                _command("NextX10"));
            _actions["NextX100"] = ftk::Action::create(
                "Forward a Second",
                _command("NextX100"));
            _actions["NextRefresh"] = ftk::Action::create(
                "Next Refresh",
                _command("NextRefresh"));
            _actions["PrevRefresh"] = ftk::Action::create(
                "Previous Refresh",
                _command("PrevRefresh"));
            _actions["FocusCurrent"] = ftk::Action::create(
                "Focus Current Frame",
                _command("FocusCurrent"));

            // Register the shortcuts.
            // To the ends by Shift+Left and Right, as in Blender; Home
            // fits the picture to the view (see ViewActions). Ten frames by
            // Alt, a second by Control.
            _addShortcut("Start", "Start", ftk::KeyShortcut(ftk::Key::Left, static_cast<int>(ftk::KeyModifier::Shift)));
            _addShortcut("End", "End", ftk::KeyShortcut(ftk::Key::Right, static_cast<int>(ftk::KeyModifier::Shift)));
            _addShortcut("Prev", "Previous", ftk::Key::Left);
            _addShortcut("PrevX10", "Previous X10", ftk::KeyShortcut(ftk::Key::Left, static_cast<int>(ftk::KeyModifier::Alt)));
            _addShortcut("PrevX100", "Back a second", ftk::KeyShortcut(ftk::Key::Left, static_cast<int>(ftk::KeyModifier::Control)));
            _addShortcut("Next", "Next", ftk::Key::Right);
            _addShortcut("NextX10", "Next X10", ftk::KeyShortcut(ftk::Key::Right, static_cast<int>(ftk::KeyModifier::Alt)));
            _addShortcut("NextX100", "Forward a second", ftk::KeyShortcut(ftk::Key::Right, static_cast<int>(ftk::KeyModifier::Control)));
            // From one part rendered again to the next by Up and Down, as
            // Blender goes from keyframe to keyframe.
            _addShortcut("NextRefresh", "Next Refresh", ftk::Key::Up);
            _addShortcut("PrevRefresh", "Previous Refresh", ftk::Key::Down);
            _addShortcut("FocusCurrent", "Focus Current", ftk::KeyShortcut(ftk::Key::F, static_cast<int>(ftk::KeyModifier::Control)));

            _shortcutsUpdate(app->getSettingsModel()->getShortcuts());

            p.playerObserver = ftk::Observer<std::shared_ptr<tl::Player> >::create(
                app->observePlayer(),
                [this](const std::shared_ptr<tl::Player>& value)
                {
                    // A file with no video is timed in audio samples, so a
                    // frame is one sample and stepping by frames goes
                    // nowhere anyone can hear. Jumping by seconds is what
                    // serves those; the ends of the file still do.
                    const bool player = value.get();
                    const bool frames =
                        player && !value->getIOInfo().video.empty();
                    _actions["Start"]->setEnabled(player);
                    _actions["End"]->setEnabled(player);
                    _actions["Prev"]->setEnabled(frames);
                    _actions["PrevX10"]->setEnabled(frames);
                    _actions["PrevX100"]->setEnabled(frames);
                    _actions["Next"]->setEnabled(frames);
                    _actions["NextX10"]->setEnabled(frames);
                    _actions["NextX100"]->setEnabled(frames);
                    _actions["NextRefresh"]->setEnabled(frames);
                    _actions["PrevRefresh"]->setEnabled(frames);
                    _actions["FocusCurrent"]->setEnabled(player);
                });
        }

        FrameActions::FrameActions() :
            _p(new Private)
        {}

        FrameActions::~FrameActions()
        {}

        std::shared_ptr<FrameActions> FrameActions::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<MainWindow>& mainWindow)
        {
            auto out = std::shared_ptr<FrameActions>(new FrameActions);
            out->_init(context, app, mainWindow);
            return out;
        }
    }
}
