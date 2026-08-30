import { useEffect } from 'react';
import type { Category, Topic } from '../types/topics';

interface TopicModalProps {
  topic: Topic | null;
  category: Category | null;
  onClose: () => void;
}

/**
 * Modal overlay showing a topic's description and its related lessons.
 * Closes on Escape or backdrop click.
 */
export function TopicModal({ topic, category, onClose }: TopicModalProps) {
  useEffect(() => {
    if (!topic) return;
    const onKey = (e: KeyboardEvent) => {
      if (e.key === 'Escape') onClose();
    };
    window.addEventListener('keydown', onKey);
    return () => window.removeEventListener('keydown', onKey);
  }, [topic, onClose]);

  if (!topic) return null;

  return (
    <div
      className="fixed inset-0 z-50 flex items-center justify-center bg-black/50 p-4"
      onClick={onClose}
      role="presentation"
    >
      <div
        className="w-full max-w-lg overflow-hidden rounded-xl bg-white shadow-xl animate-slide-up"
        onClick={(e) => e.stopPropagation()}
        role="dialog"
        aria-modal="true"
        aria-labelledby="topic-modal-title"
      >
        <header className="flex items-start justify-between border-b border-secondary-200 px-6 py-4">
          <div>
            {category && (
              <p className="text-xs font-semibold uppercase tracking-wide text-primary-600">
                {category.name}
              </p>
            )}
            <h2 id="topic-modal-title" className="text-lg font-semibold text-secondary-900">
              {topic.name}
            </h2>
          </div>
          <button
            type="button"
            onClick={onClose}
            aria-label="Schließen"
            className="flex h-8 w-8 items-center justify-center rounded-lg text-secondary-500 hover:bg-secondary-100 transition-colors"
          >
            ×
          </button>
        </header>

        <div className="max-h-[60vh] overflow-y-auto px-6 py-4 scrollbar-thin">
          <p className="text-sm leading-relaxed text-secondary-700">
            {topic.description}
          </p>

          <h3 className="mt-5 mb-2 text-sm font-semibold text-secondary-800">
            Verwandte Lektionen ({topic.lessons.length})
          </h3>
          {topic.lessons.length === 0 ? (
            <p className="text-sm text-secondary-400">
              Für dieses Thema gibt es noch keine Lektionen.
            </p>
          ) : (
            <ul className="space-y-2">
              {topic.lessons.map((lesson) => (
                <li
                  key={lesson.id}
                  className="rounded-lg border border-secondary-200 bg-secondary-50 px-3 py-2"
                >
                  <p className="text-sm font-medium text-secondary-800">
                    {lesson.title}
                  </p>
                  <p className="text-xs text-secondary-600">{lesson.summary}</p>
                </li>
              ))}
            </ul>
          )}
        </div>
      </div>
    </div>
  );
}
