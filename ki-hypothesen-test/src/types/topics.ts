// Types describing the knowledge base: categories group topics, topics own lessons.

export interface Category {
  id: string;
  name: string;
  description: string;
}

export interface Lesson {
  id: string;
  title: string;
  summary: string;
  topicId: string;
  popularity: number;
  createdAt: string;
}

export interface Topic {
  id: string;
  name: string;
  description: string;
  categoryId: string;
  lessons: Lesson[];
}
