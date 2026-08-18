import { useMemo, useState } from 'react';
import { Sidebar } from './components/Sidebar';
import { LessonsTab } from './components/LessonsTab';
import { TopicModal } from './components/TopicModal';
import { useTopics } from './hooks/useTopics';
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

  // Hypothesis state + form state
  const [hypotheses, setHypotheses] = useState<Hypothesis[]>([
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

  // Selecting a topic in the sidebar opens the modal.
  function handleSelectTopic(topicId: string) {
    setSelectedTopicId(topicId);
    setModalTopicId(topicId);
  }

  return (
    <div className="flex h-screen w-full flex-col bg-slate-100 text-slate-900">
      <header className="flex items-center justify-between border-b border-slate-200 bg-white px-6 py-3">
        <h1 className="text-lg font-semibold">KI Hypothesen Test</h1>
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
              onSubmit={handleSubmit}
              title={title}
              description={description}
              topic={topic}
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
      className={`rounded-md px-3 py-1.5 text-sm font-medium transition-colors ${
        active
          ? 'bg-white text-slate-900 shadow-sm'
          : 'text-slate-600 hover:text-slate-900'
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
  onSubmit: (e: React.FormEvent) => void;
  title: string;
  description: string;
  topic: string;
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
  onSubmit,
  title,
  description,
  topic,
  onTitle,
  onDescription,
  onTopic,
}: HypothesesPanelProps) {
  return (
    <div className="space-y-6">
      <form
        onSubmit={onSubmit}
        className="rounded-xl border border-slate-200 bg-white p-5 shadow-sm"
      >
        <h2 className="mb-3 text-base font-semibold">Neue Hypothese</h2>
        <div className="grid grid-cols-1 gap-3 sm:grid-cols-2">
          <label className="sm:col-span-2">
            <span className="mb-1 block text-xs font-medium text-slate-600">
              Titel
            </span>
            <input
              value={title}
              onChange={(e) => onTitle(e.target.value)}
              required
              className="w-full rounded-md border border-slate-300 px-3 py-2 text-sm focus:border-indigo-500 focus:outline-none focus:ring-1 focus:ring-indigo-500"
              placeholder="z. B. Mehr Daten verbessern die Genauigkeit"
            />
          </label>
          <label className="sm:col-span-2">
            <span className="mb-1 block text-xs font-medium text-slate-600">
              Beschreibung
            </span>
            <textarea
              value={description}
              onChange={(e) => onDescription(e.target.value)}
              rows={2}
              className="w-full rounded-md border border-slate-300 px-3 py-2 text-sm focus:border-indigo-500 focus:outline-none focus:ring-1 focus:ring-indigo-500"
              placeholder="Kurze Erläuterung der Hypothese"
            />
          </label>
          <label className="sm:col-span-2">
            <span className="mb-1 block text-xs font-medium text-slate-600">
              Thema
            </span>
            <select
              value={topic}
              onChange={(e) => onTopic(e.target.value)}
              required
              className="w-full rounded-md border border-slate-300 px-3 py-2 text-sm focus:border-indigo-500 focus:outline-none focus:ring-1 focus:ring-indigo-500"
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
          className="mt-3 rounded-md bg-indigo-600 px-4 py-2 text-sm font-medium text-white hover:bg-indigo-700"
        >
          Hinzufügen
        </button>
      </form>

      <section>
        <div className="mb-3 flex items-center justify-between">
          <h2 className="text-base font-semibold">Hypothesen ({hypotheses.length})</h2>
          <label className="flex items-center gap-2 text-xs text-slate-600">
            Filter:
            <select
              value={filterTopic}
              onChange={(e) => onFilterTopic(e.target.value)}
              className="rounded-md border border-slate-300 bg-white px-2 py-1 text-sm focus:border-indigo-500 focus:outline-none focus:ring-1 focus:ring-indigo-500"
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
          <p className="rounded-xl border border-dashed border-slate-300 bg-white p-6 text-center text-sm text-slate-500">
            Keine Hypothesen für diesen Filter vorhanden.
          </p>
        ) : (
          <div className="overflow-hidden rounded-xl border border-slate-200 bg-white shadow-sm">
            <table className="w-full text-sm">
              <thead className="bg-slate-50 text-left text-xs uppercase tracking-wide text-slate-500">
                <tr>
                  <th className="px-4 py-2">Titel</th>
                  <th className="px-4 py-2">Thema</th>
                  <th className="px-4 py-2">Status</th>
                  <th className="px-4 py-2">Erstellt</th>
                </tr>
              </thead>
              <tbody className="divide-y divide-slate-100">
                {hypotheses.map((h) => (
                  <tr key={h.id}>
                    <td className="px-4 py-2">
                      <p className="font-medium text-slate-900">{h.title}</p>
                      {h.description && (
                        <p className="text-xs text-slate-500">{h.description}</p>
                      )}
                    </td>
                    <td className="px-4 py-2 text-slate-700">
                      {getTopic(h.topic)?.name ?? h.topic}
                    </td>
                    <td className="px-4 py-2">
                      <select
                        value={h.status}
                        onChange={(e) =>
                          onStatusChange(h.id, e.target.value as HypothesisStatus)
                        }
                        className="rounded-md border border-slate-300 bg-white px-2 py-1 text-xs focus:border-indigo-500 focus:outline-none focus:ring-1 focus:ring-indigo-500"
                      >
                        {HYPOTHESIS_STATUSES.map((s) => (
                          <option key={s} value={s}>
                            {HYPOTHESIS_STATUS_LABELS[s]}
                          </option>
                        ))}
                      </select>
                    </td>
                    <td className="px-4 py-2 text-xs text-slate-500">
                      {new Date(h.createdAt).toLocaleString('de-DE')}
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        )}
      </section>
    </div>
  );
}
