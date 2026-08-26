import { useMemo, useState, useEffect } from 'react';
import { Sidebar } from './components/Sidebar';
import { LessonsTab } from './components/LessonsTab';
import { TopicModal } from './components/TopicModal';
import { useTopics } from './hooks/useTopics';
import { useHypothesesStorage } from './hooks/useLocalStorage';
import {
  HYPOTHESIS_STATUSES,
  HYPOTHESIS_STATUS_LABELS,
  type Hypothesis,
  type HypothesisStatus,
} from './types/hypothesis';

type Tab = 'hypotheses' | 'lessons';

export default function App() {
  const topicsApi = useTopics();
  const {
    categories,
    topics,
    popularLessons,
    recentLessons,
    getTopic,
    getCategory,
  } = topicsApi;

  const [tab, setTab] = useState<Tab>('hypotheses');
  const [sidebarCollapsed, setSidebarCollapsed] = useState(false);
  const [selectedTopicId, setSelectedTopicId] = useState<string | null>(null);
  const [modalTopicId, setModalTopicId] = useState<string | null>(null);

  // Hypothesis state + form state with localStorage persistence
  const [hypotheses, setHypotheses] = useHypothesesStorage([
    {
      id: 'seed-1',
      title: 'Few-Shot-Prompts verbessern LLM-Antworten',
      description:
        'Mit wenigen Beispielen im Prompt erzielt das Modell präzisere Antworten.',
      topic: 'topic-llms',
      status: 'testing',
      createdAt: new Date().toISOString(),
    },
  ]);
  const [filterTopic, setFilterTopic] = useState<string>('all');

  const [title, setTitle] = useState('');
  const [description, setDescription] = useState('');
  const [topic, setTopic] = useState<string>(topics[0]?.id ?? '');

  // Initialize topic state when topics load
  useEffect(() => {
    if (topics.length > 0 && !topic) {
      setTopic(topics[0].id);
    }
  }, [topics, topic]);

  const modalTopic = modalTopicId ? getTopic(modalTopicId) ?? null : null;
  const modalCategory = modalTopic ? getCategory(modalTopic.categoryId) ?? null : null;

  const filteredHypotheses = useMemo(() => {
    if (filterTopic === 'all') return hypotheses;
    return hypotheses.filter((h) => h.topic === filterTopic);
  }, [hypotheses, filterTopic]);

  function handleSubmit(e: React.FormEvent) {
    e.preventDefault();
    if (!title.trim() || !topic) return;
    const newHypothesis: Hypothesis = {
      id: `h-${crypto.randomUUID()}`,
      title: title.trim(),
      description: description.trim(),
      topic,
      status: 'draft',
      createdAt: new Date().toISOString(),
    };
    setHypotheses((prev) => [newHypothesis, ...prev]);
    setTitle('');
    setDescription('');
  }

  function handleStatusChange(id: string, status: HypothesisStatus) {
    setHypotheses((prev) =>
      prev.map((h) => (h.id === id ? { ...h, status } : h)),
    );
  }

  function handleDelete(id: string) {
    if (window.confirm('Möchten Sie diese Hypothese wirklich löschen?')) {
      setHypotheses((prev) => prev.filter((h) => h.id !== id));
    }
  }

  // Selecting a topic in the sidebar opens the modal.
  function handleSelectTopic(topicId: string) {
    setSelectedTopicId(topicId);
    setModalTopicId(topicId);
  }

  return (
    <div className="flex h-screen w-full flex-col bg-slate-50 text-secondary-900">
      <header className="flex items-center justify-between border-b border-secondary-200 bg-white px-6 py-3 shadow-soft">
        <h1 className="text-lg font-semibold text-primary-700">KI Hypothesen Test</h1>
        <nav className="flex gap-1 rounded-lg bg-slate-100 p-1">
          <TabButton active={tab === 'hypotheses'} onClick={() => setTab('hypotheses')}>
            Hypothesen
          </TabButton>
          <TabButton active={tab === 'lessons'} onClick={() => setTab('lessons')}>
            Knowledge Base
          </TabButton>
        </nav>
      </header>

      <div className="flex flex-1 overflow-hidden">
        <Sidebar
          collapsed={sidebarCollapsed}
          onToggleCollapsed={() => setSidebarCollapsed((c) => !c)}
          categories={categories}
          topics={topics}
          topicsByCategory={topicsApi.topicsByCategory}
          selectedTopicId={selectedTopicId}
          onSelectTopic={handleSelectTopic}
        />

        <main className="flex-1 overflow-y-auto p-6">
          {tab === 'hypotheses' ? (
            <HypothesesPanel
              hypotheses={filteredHypotheses}
              topics={topics}
              filterTopic={filterTopic}
              onFilterTopic={setFilterTopic}
              getTopic={getTopic}
              onStatusChange={handleStatusChange}
              onDelete={handleDelete}
              onSubmit={handleSubmit}
              title={title}
              description={description}
              selectedTopic={topic}
              onTitle={setTitle}
              onDescription={setDescription}
              onTopic={setTopic}
            />
          ) : (
            <LessonsTab
              categories={categories}
              topics={topics}
              popularLessons={popularLessons}
              recentLessons={recentLessons}
              getTopic={getTopic}
              onSelectTopic={handleSelectTopic}
            />
          )}
        </main>
      </div>

      <TopicModal
        topic={modalTopic}
        category={modalCategory}
        onClose={() => setModalTopicId(null)}
      />
    </div>
  );
}

interface TabButtonProps {
  active: boolean;
  onClick: () => void;
  children: React.ReactNode;
}

function TabButton({ active, onClick, children }: TabButtonProps) {
  return (
    <button
      type="button"
      onClick={onClick}
      className={`rounded-md px-3 py-1.5 text-sm font-medium transition-all duration-200 ${
        active
          ? 'bg-white text-primary-700 shadow-soft'
          : 'text-secondary-600 hover:bg-white/50 hover:text-primary-700'
      }`}
    >
      {children}
    </button>
  );
}

interface HypothesesPanelProps {
  hypotheses: Hypothesis[];
  topics: { id: string; name: string }[];
  filterTopic: string;
  onFilterTopic: (topicId: string) => void;
  getTopic: (id: string) => { name: string } | undefined;
  onStatusChange: (id: string, status: HypothesisStatus) => void;
  onDelete: (id: string) => void;
  onSubmit: (e: React.FormEvent) => void;
  title: string;
  description: string;
  selectedTopic: string;
  onTitle: (v: string) => void;
  onDescription: (v: string) => void;
  onTopic: (v: string) => void;
}

function HypothesesPanel({
  hypotheses,
  topics,
  filterTopic,
  onFilterTopic,
  getTopic,
  onStatusChange,
  onDelete,
  onSubmit,
  title,
  description,
  selectedTopic,
  onTitle,
  onDescription,
  onTopic,
}: HypothesesPanelProps) {
  return (
    <div className="space-y-6">
      <form
        onSubmit={onSubmit}
        className="card card-hover"
      >
        <h2 className="mb-3 text-base font-semibold text-primary-700">Neue Hypothese</h2>
        <div className="grid grid-cols-1 gap-3 sm:grid-cols-2">
          <label className="sm:col-span-2">
            <span className="mb-1 block text-xs font-medium text-secondary-600">
              Titel
            </span>
            <input
              value={title}
              onChange={(e) => onTitle(e.target.value)}
              required
              className="input-field"
              placeholder="z. B. Mehr Daten verbessern die Genauigkeit"
            />
          </label>
          <label className="sm:col-span-2">
            <span className="mb-1 block text-xs font-medium text-secondary-600">
              Beschreibung
            </span>
            <textarea
              value={description}
              onChange={(e) => onDescription(e.target.value)}
              rows={2}
              className="input-field"
              placeholder="Kurze Erläuterung der Hypothese"
            />
          </label>
          <label className="sm:col-span-2">
            <span className="mb-1 block text-xs font-medium text-secondary-600">
              Thema
            </span>
            <select
              value={selectedTopic}
              onChange={(e) => onTopic(e.target.value)}
              required
              className="input-field"
            >
              {topics.map((t) => (
                <option key={t.id} value={t.id}>
                  {t.name}
                </option>
              ))}
            </select>
          </label>
        </div>
        <button
          type="submit"
          className="btn-primary mt-3"
        >
          Hinzufügen
        </button>
      </form>

      <section>
        <div className="mb-3 flex flex-col sm:flex-row sm:items-center sm:justify-between gap-2">
          <h2 className="text-base font-semibold text-secondary-900">
            Hypothesen ({hypotheses.length})
          </h2>
          <label className="flex items-center gap-2 text-xs text-secondary-600">
            Filter:
            <select
              value={filterTopic}
              onChange={(e) => onFilterTopic(e.target.value)}
              className="input-field text-xs"
            >
              <option value="all">Alle Themen</option>
              {topics.map((t) => (
                <option key={t.id} value={t.id}>
                  {t.name}
                </option>
              ))}
            </select>
          </label>
        </div>

        {hypotheses.length === 0 ? (
          <p className="card text-center text-sm text-secondary-500">
            Keine Hypothesen für diesen Filter vorhanden.
          </p>
        ) : (
          <div className="overflow-hidden rounded-xl border border-secondary-200 bg-white shadow-soft">
            <table className="w-full text-sm">
              <thead className="bg-secondary-50 text-left text-xs uppercase tracking-wide text-secondary-500">
                <tr>
                  <th className="px-4 py-3">Titel</th>
                  <th className="px-4 py-3">Thema</th>
                  <th className="px-4 py-3">Status</th>
                  <th className="px-4 py-3">Erstellt</th>
                  <th className="px-4 py-3">Aktionen</th>
                </tr>
              </thead>
              <tbody className="divide-y divide-secondary-100">
                {hypotheses.map((h) => {
                  const topicName = getTopic(h.topic)?.name ?? h.topic;
                  return (
                    <tr key={h.id} className="animate-fade-in">
                      <td className="px-4 py-3">
                        <p className="font-medium text-secondary-900">{h.title}</p>
                        {h.description && (
                          <p className="text-xs text-secondary-500">{h.description}</p>
                        )}
                      </td>
                      <td className="px-4 py-3 text-secondary-700">
                        <span className="badge badge-secondary">{topicName}</span>
                      </td>
                      <td className="px-4 py-3">
                        <select
                          value={h.status}
                          onChange={(e) =>
                            onStatusChange(h.id, e.target.value as HypothesisStatus)
                          }
                          className="rounded-md border border-secondary-300 bg-white px-2 py-1 text-xs focus:border-primary-500 focus:outline-none focus:ring-1 focus:ring-primary-500"
                        >
                          {HYPOTHESIS_STATUSES.map((s) => (
                            <option key={s} value={s}>
                              {HYPOTHESIS_STATUS_LABELS[s]}
                            </option>
                          ))}
                        </select>
                      </td>
                      <td className="px-4 py-3 text-xs text-secondary-500">
                        {new Date(h.createdAt).toLocaleString('de-DE')}
                      </td>
                      <td className="px-4 py-3">
                        <button
                          onClick={() => onDelete(h.id)}
                          className="rounded-md px-2 py-1 text-xs font-medium text-red-600 hover:bg-red-50 hover:text-red-700 transition-colors"
                          title="Löschen"
                        >
                          Löschen
                        </button>
                      </td>
                    </tr>
                  );
                })}
              </tbody>
            </table>
          </div>
        )}
      </section>
    </div>
  );
}
