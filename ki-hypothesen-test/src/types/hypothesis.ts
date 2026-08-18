// A single testable hypothesis entry created by the user.

export interface Hypothesis {
  id: string;
  title: string;
  description: string;
  /** Topic id (see topics.ts) this hypothesis belongs to. */
  topic: string;
  status: HypothesisStatus;
  createdAt: string;
}

export type HypothesisStatus = 'draft' | 'testing' | 'confirmed' | 'refuted';

export const HYPOTHESIS_STATUSES: readonly HypothesisStatus[] = [
  'draft',
  'testing',
  'confirmed',
  'refuted',
] as const;

export const HYPOTHESIS_STATUS_LABELS: Record<HypothesisStatus, string> = {
  draft: 'Entwurf',
  testing: 'Wird getestet',
  confirmed: 'Bestätigt',
  refuted: 'Widerlegt',
};
