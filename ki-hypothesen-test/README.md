# KI Hypothesen Test

React + Vite + TypeScript SPA for recording and managing testable AI hypotheses,
with a knowledge-base sidebar, lessons tab, and topic detail modal.

This subfolder is independent of the parent Kodi visualization addon (C++).

## Getting started

```bash
cd ki-hypothesen-test
npm install
npm run dev      # start dev server on http://localhost:5173/
npm run build    # type-check (tsc -b) + production build (vite build)
npm run preview  # preview the production build
```

## Features

- **Sidebar** — collapsible, searchable list of categories and topics.
- **Knowledge Base tab** — category cards, popular lessons, and recent lessons.
- **Topic Modal** — click any topic to see its description and related lessons.
- **Hypothesis form** — add a hypothesis with a title, description, and topic.
- **Hypothesis table** — filter by topic and update status inline.

## Structure

```
src/
├── components/
│   ├── Sidebar.tsx       # Collapsible, searchable topics sidebar
│   ├── LessonsTab.tsx    # Knowledge base with categories + popular/recent lessons
│   └── TopicModal.tsx    # Modal for topic details
├── data/
│   └── topics.json       # Static categories/topics/lessons data
├── hooks/
│   └── useTopics.ts      # Custom hook exposing topic data + derived views
├── types/
│   ├── hypothesis.ts     # Hypothesis type (incl. `topic` field) + statuses
│   └── topics.ts         # Category / Topic / Lesson types
├── App.tsx               # Tabs, sidebar, form + table, modal wiring
├── index.css             # Tailwind directives
└── main.tsx              # React entry point
```
