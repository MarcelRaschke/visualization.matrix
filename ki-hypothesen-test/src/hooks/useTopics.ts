import { useMemo } from 'react';
import topicsData from '../data/topics.json';
import type { Category, Lesson, Topic } from '../types/topics';

interface TopicsDataset {
  categories: Category[];
  topics: Topic[];
}

const dataset = topicsData as TopicsDataset;

export interface UseTopicsResult {
  categories: Category[];
  topics: Topic[];
  /** Flattened list of all lessons across every topic. */
  lessons: Lesson[];
  /** Lessons sorted by descending popularity. */
  popularLessons: Lesson[];
  /** Lessons sorted by descending creation date. */
  recentLessons: Lesson[];
  getTopic: (id: string) => Topic | undefined;
  getCategory: (id: string) => Category | undefined;
  topicsByCategory: (categoryId: string) => Topic[];
}

/**
 * Custom hook that exposes the static knowledge-base data together with
 * a few convenient derived views (popular/recent lessons, lookups).
 */
export function useTopics(): UseTopicsResult {
  const { categories, topics } = dataset;

  return useMemo<UseTopicsResult>(() => {
    const lessons: Lesson[] = topics.flatMap((t) => t.lessons);

    const popularLessons = [...lessons].sort(
      (a, b) => b.popularity - a.popularity,
    );

    const recentLessons = [...lessons].sort(
      (a, b) => +new Date(b.createdAt) - +new Date(a.createdAt),
    );

    const topicById = new Map<string, Topic>(topics.map((t) => [t.id, t]));
    const categoryById = new Map<string, Category>(
      categories.map((c) => [c.id, c]),
    );

    return {
      categories,
      topics,
      lessons,
      popularLessons,
      recentLessons,
      getTopic: (id: string) => topicById.get(id),
      getCategory: (id: string) => categoryById.get(id),
      topicsByCategory: (categoryId: string) =>
        topics.filter((t) => t.categoryId === categoryId),
    };
  }, [categories, topics]);
}
