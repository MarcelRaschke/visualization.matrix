import { useState, useEffect } from 'react';
import type { Hypothesis } from '../types/hypothesis';

/**
 * Generic hook for managing state with localStorage persistence.
 * @param key - The localStorage key to use
 * @param initialValue - The initial value if nothing is stored
 * @returns A stateful value and a function to update it
 */
export function useLocalStorage<T>(key: string, initialValue: T): [T, (value: T | ((val: T) => T)) => void] {
  const [storedValue, setStoredValue] = useState<T>(() => {
    try {
      const item = window.localStorage.getItem(key);
      return item ? JSON.parse(item) : initialValue;
    } catch (error) {
      console.error('Error reading from localStorage:', error);
      return initialValue;
    }
  });

  useEffect(() => {
    try {
      window.localStorage.setItem(key, JSON.stringify(storedValue));
    } catch (error) {
      console.error('Error saving to localStorage:', error);
    }
  }, [key, storedValue]);

  return [storedValue, setStoredValue];
}

/**
 * Hook specifically for managing hypotheses in localStorage.
 * Handles the hypotheses array with proper typing.
 */
const STORAGE_KEY = 'ki-hypothesen-test-hypotheses';

export function useHypothesesStorage(
  initialHypotheses: Hypothesis[] = [],
): [Hypothesis[], (value: Hypothesis[] | ((val: Hypothesis[]) => Hypothesis[])) => void] {
  return useLocalStorage<Hypothesis[]>(STORAGE_KEY, initialHypotheses);
}
