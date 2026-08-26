import type { Category, Lesson, Topic } from '../types/topics';

interface LessonsTabProps {
  categories: Category[];
  topics: Topic[];
  popularLessons: Lesson[];
  recentLessons: Lesson[];
  getTopic: (id: string) => Topic | undefined;
  onSelectTopic: (topicId: string) => void;
}

/**
 * Knowledge-base tab: category cards, popular lessons and recent lessons,
 * each rendered in a responsive grid. Clicking a topic opens the modal.
 */
export function LessonsTab({
  categories,
  topics,
  popularLessons,
  recentLessons,
  getTopic,
  onSelectTopic,
}: LessonsTabProps) {
  return (
    <div className="space-y-8">
      <section>
        <h2 className="mb-3 text-lg font-semibold text-secondary-900">Kategorien</h2>
        <div className="grid grid-cols-1 gap-4 sm:grid-cols-2 lg:grid-cols-3">
          {categories.map((cat) => (
            <div
              key={cat.id}
              className="card card-hover"
            >
              <h3 className="text-sm font-semibold text-primary-700">{cat.name}</h3>
              <p className="mt-1 text-xs text-secondary-600">{cat.description}</p>
              <ul className="mt-3 space-y-1">
                {topics
                  .filter((t) => t.categoryId === cat.id)
                  .map((topic) => (
                    <li key={topic.id}>
                      <button
                        type="button"
                        onClick={() => onSelectTopic(topic.id)}
                        className="text-sm text-primary-600 hover:underline"
                      >
                        {topic.name}
                      </button>
                    </li>
                  ))}
              </ul>
            </div>
          ))}
        </div>
      </section>

      <LessonGrid
        title="Beliebte Lektionen"
        lessons={popularLessons.slice(0, 6)}
        getTopic={getTopic}
        onSelectTopic={onSelectTopic}
        metric={(l) => `Beliebtheit ${l.popularity}`}
      />

      <LessonGrid
        title="Neueste Lektionen"
        lessons={recentLessons.slice(0, 6)}
        getTopic={getTopic}
        onSelectTopic={onSelectTopic}
        metric={(l) => formatDate(l.createdAt)}
      />
    </div>
  );
}

interface LessonGridProps {
  title: string;
  lessons: Lesson[];
  getTopic: (id: string) => Topic | undefined;
  onSelectTopic: (topicId: string) => void;
  metric: (lesson: Lesson) => string;
}

function LessonGrid({
  title,
  lessons,
  getTopic,
  onSelectTopic,
  metric,
}: LessonGridProps) {
  return (
    <section>
      <h2 className="mb-3 text-lg font-semibold text-secondary-900">{title}</h2>
      <div className="grid grid-cols-1 gap-4 sm:grid-cols-2 lg:grid-cols-3">
        {lessons.map((lesson) => {
          const topic = getTopic(lesson.topicId);
          return (
            <article
              key={lesson.id}
              className="card card-hover"
            >
              <p className="text-xs font-semibold uppercase tracking-wide text-primary-600">
                {metric(lesson)}
              </p>
              <h3 className="mt-1 text-sm font-semibold text-secondary-900">
                {lesson.title}
              </h3>
              <p className="mt-1 flex-1 text-xs text-secondary-600">{lesson.summary}</p>
              {topic && (
                <button
                  type="button"
                  onClick={() => onSelectTopic(topic.id)}
                  className="mt-3 self-start badge badge-primary"
                >
                  Thema: {topic.name}
                </button>
              )}
            </article>
          );
        })}
      </div>
    </section>
  );
}

function formatDate(iso: string): string {
  const d = new Date(iso);
  if (Number.isNaN(d.getTime())) return iso;
  return d.toLocaleDateString('de-DE', {
    day: '2-digit',
    month: '2-digit',
    year: 'numeric',
  });
}
