// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the DJV project.

#include <djv/UI/WrapLabel.h>

#include <djv/UI/Fluid.h>

#include <ftk/Core/FontSystem.h>

#include <algorithm>
#include <sstream>

namespace djv
{
    namespace ui
    {
        namespace
        {
            std::vector<std::string> wrap(
                const std::string& text,
                int width,
                const std::shared_ptr<ftk::FontSystem>& fontSystem,
                const ftk::FontInfo& fontInfo)
            {
                std::vector<std::string> out;
                std::stringstream paragraphs(text);
                std::string paragraph;
                while (std::getline(paragraphs, paragraph, '\n'))
                {
                    std::stringstream words(paragraph);
                    std::string word;
                    std::string line;
                    while (words >> word)
                    {
                        const std::string candidate = line.empty() ? word : line + " " + word;
                        if (!line.empty() && fontSystem->getSize(candidate, fontInfo).w > width)
                        {
                            out.push_back(line);
                            line = word;
                        }
                        else
                        {
                            line = candidate;
                        }
                    }
                    out.push_back(line);
                }
                return out;
            }
        }

        struct WrapLabel::Private
        {
            std::string text;
            ftk::SizeRole marginRole = ftk::SizeRole::None;
            int width = 0;

            int margin = 0;
            ftk::FontInfo fontInfo;
            ftk::FontMetrics fontMetrics;
            std::vector<std::string> lines;
            ftk::Size2I sizeHint;
        };

        void WrapLabel::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::string& text,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "djv::ui::WrapLabel", parent);
            _p->text = text;
            setHStretch(ftk::Stretch::Expanding);
        }

        WrapLabel::WrapLabel() :
            _p(new Private)
        {}

        WrapLabel::~WrapLabel()
        {}

        std::shared_ptr<WrapLabel> WrapLabel::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            return create(context, std::string(), parent);
        }

        std::shared_ptr<WrapLabel> WrapLabel::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::string& text,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<WrapLabel>(new WrapLabel);
            out->_init(context, text, parent);
            return out;
        }

        const std::string& WrapLabel::getText() const
        {
            return _p->text;
        }

        void WrapLabel::setText(const std::string& value)
        {
            FTK_P();
            if (value == p.text)
                return;
            p.text = value;
            setSizeUpdate();
            setDrawUpdate();
        }

        void WrapLabel::setMarginRole(ftk::SizeRole value)
        {
            FTK_P();
            if (value == p.marginRole)
                return;
            p.marginRole = value;
            setSizeUpdate();
            setDrawUpdate();
        }

        ftk::Size2I WrapLabel::getSizeHint() const
        {
            return _p->sizeHint;
        }

        void WrapLabel::setGeometry(const ftk::Box2I& value)
        {
            IWidget::setGeometry(value);
            FTK_P();
            // The lines follow the width: measured again at the next size
            // pass, which then gives the height they need.
            if (value.w() != p.width)
            {
                p.width = value.w();
                setSizeUpdate();
                setDrawUpdate();
            }
        }

        void WrapLabel::sizeHintEvent(const ftk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            FTK_P();
            const float s = event.displayScale;
            p.margin = event.style->getSizeRole(p.marginRole, s);
            p.fontInfo = event.style->getFont(ftk::FontType::Regular, s);
            p.fontMetrics = event.fontSystem->getMetrics(p.fontInfo);
            const int minWidth = getFluidMinWidth(s);
            const int width = std::max(p.width, minWidth) - p.margin * 2;
            p.lines = wrap(p.text, std::max(1, width), event.fontSystem, p.fontInfo);
            if (p.text.empty())
            {
                p.lines.clear();
            }
            p.sizeHint = ftk::Size2I(
                minWidth,
                static_cast<int>(p.lines.size()) * p.fontMetrics.lineHeight + p.margin * 2);
        }

        void WrapLabel::drawEvent(const ftk::Box2I& drawRect, const ftk::DrawEvent& event)
        {
            IWidget::drawEvent(drawRect, event);
            FTK_P();
            const ftk::Box2I& g = getGeometry();
            int y = g.min.y + p.margin;
            for (const auto& line : p.lines)
            {
                event.render->drawText(
                    event.fontSystem->getGlyphs(line, p.fontInfo),
                    p.fontMetrics,
                    ftk::V2I(g.min.x + p.margin, y),
                    event.style->getColorRole(ftk::ColorRole::Text, isEnabled()));
                y += p.fontMetrics.lineHeight;
            }
        }
    }
}
