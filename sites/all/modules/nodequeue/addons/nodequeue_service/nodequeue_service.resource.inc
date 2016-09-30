<?php
/**
 * @file
 * Link general nodequeue functionalities to services module.
 */

/**
 * Callback to retrieve an array of available nodequeue objects.
 *
 * @return $queues
 *   An array of available nodequeue objects.
 */
function _nodequeue_service_retrieve_queues() {
  $queues = nodequeue_load_queues(nodequeue_get_all_qids(0));
  return $queues;
}

/**
 * Callback to retrieve an array of available sub-nodequeue objects.
 *
 * @param $qid
 *   A parent nodequeue queue ID.
 *
 * @return $subqueues
 *   An array of available sub-nodequeue objects.
 */
function _nodequeue_service_retrieve_subqueues($qid) {
  $subqueues = nodequeue_load_subqueues_by_queue($qid);
  return $subqueues;
}

/**
 * Callback to retrieve an array of all nodequeue nodes.
 *
 * @param $qid
 *   A nodequeue queue ID.
 *
 * @return $nodes
 *   An array of available node IDs.
 */
function _nodequeue_service_retrieve_queue_items($qid) {
  $query = db_select('nodequeue_nodes', 'n')
    ->fields('n', array('nid', 'position'))
    ->condition('n.sqid', $qid)
    ->orderBy('n.position', 'ASC');
  $nodes = $query->execute()->fetchAllAssoc('n.position');

  return $nodes;
}

/**
 * Callback to add a node to a nodequeue.
 *
 * @param $qid
 *   Nodequeue queue ID.
 * @param $nid
 *   Node ID of the node you want to add.
 */
function _nodequeue_service_add_queue_item($qid, $nid) {
  $subqueue = nodequeue_load_subqueue($qid);
  $queue = nodequeue_load($subqueue->qid);

  nodequeue_subqueue_add($queue, $subqueue, $nid);
}

/**
 * Callback to remove a node from a nodequeue.
 *
 * @param $qid
 *   Nodequeue queue ID.
 * @param $nid
 *   Node ID of the node you want to remove.
 */
function _nodequeue_service_remove_queue_item($qid, $nid) {
  $subqueue = nodequeue_load_subqueue($qid);
  $queue = nodequeue_load($subqueue->qid);
  $node = node_load($nid);

  if ($node) {
    if (!$queue->i18n || (isset($node->tnid) && empty($node->tnid))) {
      nodequeue_subqueue_remove_node($subqueue->qid, $node->nid);
    }
  }
}

/**
 * Callback to retrieve a node at a specific position of a nodequeue.
 *
 * @param $qid
 *   Nodequeue queue ID.
 * @param $position
 *   Position of the node in the queue you want to retrieve.
 *
 * @return $node
 *   The requested node object.
 */
function _nodequeue_service_get_queue_item($qid, $position) {
  $query = db_select('nodequeue_nodes', 'n');
  $query->addField('n', 'nid');
  $query->condition('n.sqid', $qid);
  $query->condition('n.position', $position);
  $nid = $query->execute()->fetchField();

  $node = node_load($nid);

  if ($node) {
    $uri = entity_uri('node', $node);
    $node->path = url($uri['path'], array('absolute' => TRUE));
    // Unset uri as it has complete entity and this
    // cause never ending recursion in rendering.
    unset($node->uri);
  }

  return $node;
}

/**
 * Callback to shift and return the next item of a nodequeue.
 *
 * @param $qid
 *   Nodequeue queue ID.
 *
 * @return $node
 *   The next node object.
 */
function _nodequeue_service_shift_queue_item($qid) {
  $query = db_select('nodequeue_nodes', 'n')
    ->fields('n', array('nid'))
    ->condition('n.sqid', $qid)
    ->condition('n.position', 1);
  $nid = $query->execute()->fetchField();

  $subqueue = nodequeue_load_subqueue($qid);
  $queue = nodequeue_load($subqueue->qid);
  $node = node_load($nid);

  if ($node) {

    if (!$queue->i18n || (isset($node->tnid) && empty($node->tnid))) {
      nodequeue_subqueue_remove_node($subqueue->qid, $node->nid);
    }

    $uri = entity_uri('node', $node);
    $node->path = url($uri['path'], array('absolute' => TRUE));
    // Unset uri as it has complete entity and this
    // cause never ending recursion in rendering.
    unset($node->uri);
  }

  return $node;
}
