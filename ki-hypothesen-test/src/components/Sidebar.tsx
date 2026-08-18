import { useMemo, useState } from 'react';
import type { Category, Topic } from '../types/topics';

interface SidebarProps {
  collapsed: boolean;
  onToggleCollapsed: () => void;
  categories: Category[];
  topics: Topic[];
  topicsByCategory: (categoryId: string) => Topic[];
  selectedTopicId: string | null;
  onSelectTopic: (topicId: string) => void;
}

/**
 * Collapsible, searchable sidebar listing all categories and their topics.
 * Selecting a topic opens the topic modal (handled by the parent).
 */
export function Sidebar({
  collapsed,
  onToggleCollapsed,
  categories,
  topics,
  topicsByCategory,
  selectedTopicId,
  onSelectTopic,
}: SidebarProps) {
  const [query, setQuery] = useState('');

  const matches = (t: Topic, q: string) =>
    t.name.toLowerCase().includes(q) ||
    t.description.toLowerCase().includes(q);

  const isSearching = query.trim().length > 0;
  const q = query.trim().toLowerCase();

  const filteredCategories = useMemo(() => {
    if (!isSearching) return categories;
    return categories.filter((cat) =>
      topicsByCategory(cat.id).some((t) => matches(t, q)),
    );
  }, [q, isSearching, categories, topicsByCategory]);

  if (collapsed) {
    return (
      <aside className="flex w-14 flex-col border-r border-slate-200 bg-slate-50 p-2">
        <button
          type="button"
          onClick={onToggleCollapsed}
          aria-label="Sidebar aufklappen"
          className="flex h-9 w-10 items-center justify-center rounded-md text-slate-600 hover:bg-slate-200"
        >
          »
        </button>
        <span className="mt-2 rotate-180 text-center text-xs font-medium text-slate-500 [writing-mode:vertical-rl]">
          Themen
        </span>
      </aside>
    );
  }

  return (
    <aside className="flex w-72 flex-col border-r border-slate-200 bg-slate-50">
      <div className="flex items-center justify-between border-b border-slate-200 px-4 py-3">
        <h2 className="text-sm font-semibold text-slate-800">Themen</h2>
        <button
          type="button"
          onClick={onToggleCollapsed}
          aria-label="Sidebar einklappen"
          className="flex h-7 w-7 items-center justify-center rounded-md text-slate-600 hover:bg-slate-200"
        >
          «
        </button>
      </div>

      <div className="border-b border-slate-200 p-3">
        <input
          type="search"
          value={query}
          onChange={(e) => setQuery(e.target.value)}
          placeholder="Themen durchsuchen…"
          className="w-full rounded-md border border-slate-300 bg-white px-3 py-1.5 text-sm text-slate-800 placeholder:text-slate-400 focus:border-indigo-500 focus:outline-none focus:ring-1 focus:ring-indigo-500"
        />
      </div>

      <nav className="flex-1 overflow-y-auto px-2 py-2">
        {filteredCategories.length === 0 && (
          <p className="px-2 py-4 text-sm text-slate-400">
            Keine Themen gefunden.
          </p>
        )}

        {filteredCategories.map((cat) => {
          const catTopics = isSearching
            ? topicsByCategory(cat.id).filter((t) => matches(t, q))
            : topicsByCategory(cat.id);
          return (
            <div key={cat.id} className="mb-3">
              <p className="px-2 py-1 text-xs font-semibold uppercase tracking-wide text-slate-500">
                {cat.name}
              </p>
              <ul className="space-y-0.5">
                {catTopics.map((topic) => {
                  const active = topic.id === selectedTopicId;
                  const lessonCount =
                    topics.find((t) => t.id === topic.id)?.lessons.length ?? 0;
                  return (
                    <li key={topic.id}>
                      <button
                        type="button"
                        onClick={() => onSelectTopic(topic.id)}
                        className={`w-full rounded-md px-2 py-1.5 text-left text-sm transition-colors ${
                          active
                            ? 'bg-indigo-100 text-indigo-900'
                            : 'text-slate-700 hover:bg-slate-200'
                        }`}
                      >
                        {topic.name}
                        <span className="ml-1 text-xs text-slate-400">
                          ({lessonCount})
                        </span>
                      </button>
                    </li>
                  );
                })}
              </ul>
            </div>
          );
        })}
      </nav>
    </aside>
  );
}
