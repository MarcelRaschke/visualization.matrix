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
      <aside className="flex w-14 flex-col border-r border-secondary-200 bg-slate-50 p-2 shadow-soft">
        <button
          type="button"
          onClick={onToggleCollapsed}
          aria-label="Sidebar aufklappen"
          className="flex h-9 w-10 items-center justify-center rounded-lg text-secondary-600 hover:bg-primary-50 hover:text-primary-700 transition-colors"
        >
          b
        </button>
        <span className="mt-2 rotate-180 text-center text-xs font-medium text-secondary-500 [writing-mode:vertical-rl]">
          Themen
        </span>
      </aside>
    );
  }

  return (
    <aside className="flex w-72 flex-col border-r border-secondary-200 bg-white shadow-soft">
      <div className="flex items-center justify-between border-b border-secondary-200 px-4 py-3">
        <h2 className="text-sm font-semibold text-primary-700">Themen</h2>
        <button
          type="button"
          onClick={onToggleCollapsed}
          aria-label="Sidebar einklappen"
          className="flex h-7 w-7 items-center justify-center rounded-lg text-secondary-600 hover:bg-secondary-100 hover:text-primary-700 transition-colors"
        >
          b
        </button>
      </div>

      <div className="border-b border-secondary-200 p-3">
        <input
          type="search"
          value={query}
          onChange={(e) => setQuery(e.target.value)}
          placeholder="Themen durchsuchen…"
          className="input-field"
        />
      </div>

      <nav className="flex-1 overflow-y-auto px-2 py-2 scrollbar-thin">
        {filteredCategories.length === 0 && (
          <p className="px-2 py-4 text-sm text-secondary-400">
            Keine Themen gefunden.
          </p>
        )}

        {filteredCategories.map((cat) => {
          const catTopics = isSearching
            ? topicsByCategory(cat.id).filter((t) => matches(t, q))
            : topicsByCategory(cat.id);
          return (
            <div key={cat.id} className="mb-3">
              <p className="px-2 py-1 text-xs font-semibold uppercase tracking-wide text-secondary-500">
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
                        className={`sidebar-link w-full justify-between ${active ? 'sidebar-link-active' : ''}`}
                      >
                        <span className="truncate">{topic.name}</span>
                        <span className="text-xs text-secondary-400">
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
