<?php
/**
 * @author Maxim Bogdanov <sin666m4a1fox@gmail.com>
 * @copyright maxim 4/3/14 | 3:54 PM
 */

define('SMARTLING_DEMO_MODULE', 'smartling_demo_content');

define('TRAVEL_TAXONOMY_ADDED', 'Travel taxonomy has been added to DB.');
define('DEMO_USERS_ADDED', 'Demo users has been added to DB.');
define('DEMO_USERS_EXIST', 'Demo users already exist. has been added to DB.');
define('NODES_COMMENTS_ADDED', 'Comments has been added to travel nodes.');
define('NODES_COMMENT_DELETE', 'The comments has been deleted.');
define('NO_NODES_DEMO_COMMENTS', 'No demo comments in DB.');
define('DELETE_ADDITIONAL_ACCOUNT_FIELDS', 'The additional account fields has been delete from DB');
define('DELETE_DEMO_USERS', 'Demo Users has been deleted');
define('TRAVEL_VOCABULARY_DELETED', 'The travel taxonomy has been removed from DB.');
define('TRAVEL_VOCABULARY_NOT_FOUND', 'The travel taxonomy not found in taxonomy vocabularies.');
define('TRAVEL_TAXONOMY_ADDED_TO_NODE', 'The travel taxonomy added to travel nodes randomly.');
define('TAXONOMY_TRAVEL', 'travel');
define('DEMO_LANGUAGE_DEFAULT', language_default()->language);

function smartling_modules_enabled($modules)
{
    if (in_array(SMARTLING_DEMO_MODULE, $modules)) {
        drupal_cron_run();

        _create_comments_to_nodes();
        _create_additional_fields_for_account();
        _create_taxonomy();
        //_add_taxonomy_to_travel_node();

        db_query("UPDATE {block} SET region = :region, status = :status WHERE delta = :delta AND module = :module", array(
          ':region' => 'sidebar_first',
          ':status' => 1,
          ':delta' => 'language_content',
          ':module' => 'locale',
        ));
    }
}

function smartling_modules_disabled($modules)
{
    if (in_array(SMARTLING_DEMO_MODULE, $modules)) {
        _delete_comments_to_nodes();
        _delete_additional_fields_for_account();
        _delete_taxonomy();
    }
}

function _create_comments_to_nodes()
{
    module_load_include('inc', 'devel_generate', 'devel_generate');
    $nodes = node_load_multiple(array(), array('type' => array('article', 'travel')));
    foreach ($nodes as $node) {
        $load_node = node_load($node->vid);
        if ($load_node->comment_count < 1) {
            $comment = (object)array(
                'nid' => $load_node->vid,
                'cid' => 0,
                'pid' => 0,
                'uid' => 1,
                'mail' => '',
                'is_anonymous' => 0,
                'homepage' => '',
                'status' => COMMENT_PUBLISHED,
                'subject' => devel_create_greeking(mt_rand(1, 3), TRUE),
                'language' => DEMO_LANGUAGE_DEFAULT,
                'comment_body' => array(
                    DEMO_LANGUAGE_DEFAULT => array(
                        0 => array(
                            'value' => devel_create_greeking(mt_rand(2, 17), TRUE),
                            'format' => 'filtered_html'
                        )
                    )
                ),
            );
            comment_submit($comment);
            comment_save($comment);
        }
    }

    set_message_watchdog(NODES_COMMENTS_ADDED);
}

function _delete_comments_to_nodes()
{
    $nodes = node_load_multiple(array(), array('type' => array('article', 'travel')));
    $results = db_select('comment', 'c')
        ->fields('c', array('cid', 'uid'))
        ->execute()->fetchAll();
    $comment_id = array();
    foreach ($results as $result) {
        if (array_key_exists($result->uid, $nodes)) {
            $comment_id[] = $result->cid;
        }
    }

    if (count($comment_id) > 0) {
        comment_delete_multiple($comment_id);
        set_message_watchdog(NODES_COMMENT_DELETE);
    } else {
        set_message_watchdog(NO_NODES_DEMO_COMMENTS);
    }
}

function _create_additional_fields_for_account()
{
    $data_fields = array(
        array(
            'field' => array(
                'field_name' => 'field_nick_name_field',
                'type' => 'text',
            ),
            'instance' => array(
                'field_name' => 'field_nick_name_field',
                'entity_type' => 'user',
                'label' => 'Nick Name',
                'bundle' => 'user',
                'settings' => array(
                    'user_register_form' => 1,
                ),
                'widget' => array(
                    'type' => 'textfield',
                    'weight' => '1',
                ),
            ),
        ),

        array(
            'field' => array(
                'field_name' => 'field_about_me_field',
                'type' => 'text',
            ),
            'instance' => array(
                'field_name' => 'field_about_me_field',
                'entity_type' => 'user',
                'label' => 'About Me',
                'bundle' => 'user',
                'settings' => array(
                    'user_register_form' => 1,
                ),
                'widget' => array(
                    'type' => 'textfield',
                    'weight' => '1',
                ),
            ),
        ),

        array(
            'field' => array(
                'field_name' => 'field_horoscope_field',
                'type' => 'text',
            ),
            'instance' => array(
                'field_name' => 'field_horoscope_field',
                'entity_type' => 'user',
                'label' => 'Horoscope',
                'bundle' => 'user',
                'settings' => array(
                    'user_register_form' => 1,
                ),
                'widget' => array(
                    'type' => 'textfield',
                    'weight' => '1',
                ),
            ),
        ),
    );

    $data_demo = array(
        'horoscope' => array('table' => 'field_data_field_horoscope_field', 'field' => 'field_horoscope_field_value'),
        'about_me' => array('table' => 'field_data_field_about_me_field', 'field' => 'field_about_me_field_value'),
        'nick_name' => array('table' => 'field_data_field_nick_name_field', 'field' => 'field_nick_name_field_value'),
    );

    $data_user = array(
        array(
            'fields' => array(
                'name' => 'john_doe',
                'mail' => 'john_doe@test.com',
                'pass' => user_password(8),
                'status' => 1,
                'init' => 'email address',
                'roles' => array(
                    DRUPAL_AUTHENTICATED_RID => 'authenticated user',
                ),
            ),
            'demo_data' => array(
                'horoscope' => 'Libra',
                'about_me' => 'This is description of John Doe male user',
                'nick_name' => 'John Doe',
            ),
        ),
        array(
            'fields' => array(
                'name' => 'jane_roe',
                'mail' => 'jane_roe@test.com',
                'pass' => user_password(8),
                'status' => 1,
                'init' => 'email address',
                'roles' => array(
                    DRUPAL_AUTHENTICATED_RID => 'authenticated user',
                ),
            ),
            'demo_data' => array(
                'horoscope' => 'Aquarius',
                'about_me' => 'This is description of Jane Roe female user',
                'nick_name' => 'Jane Roe',
            ),
        ),
    );

    foreach ($data_fields as $value) {
        if (!field_info_field($value['field']['field_name'])) {
            field_create_field($value['field']);
            field_create_instance($value['instance']);
        }
    }

    foreach ($data_user as $value) {
        $load_user = user_load_by_name($value['fields']['name']);
        if (!is_object($load_user)) {
            $account = user_save('', $value['fields']);
            foreach ($data_demo as $key => $val) {
                db_insert($val['table'])
                    ->fields(
                        array(
                            'entity_type' => 'user',
                            'bundle' => 'user',
                            'deleted' => 0,
                            'entity_id' => $account->uid,
                            'language' => 'und',
                            'delta' => 0,
                            $val['field'] => $value['demo_data'][$key],
                        )
                    )
                    ->execute();
            }
        } else {
            set_message_watchdog(DEMO_USERS_EXIST);
            return;
        }
    }

    set_message_watchdog(DEMO_USERS_ADDED);
}

function _delete_additional_fields_for_account()
{

    $data_fields = array(
        array(
            'field' => 'field_nick_name_field'
        ),
        array(
            'field' => 'field_about_me_field',
        ),
        array(
            'field' => 'field_horoscope_field',
        ),
    );

    $data_user = array(
        array(
            'name' => 'john_doe',
        ),
        array(
            'name' => 'jane_roe'
        ),
    );

    foreach ($data_fields as $value) {
        if (field_info_field($value['field'])) {
            field_delete_field($value['field']);
        }
    }

    foreach ($data_user as $val) {
        $user_load = user_load_by_name($val['name']);
        user_delete($user_load->uid);
    }

    set_message_watchdog(DELETE_ADDITIONAL_ACCOUNT_FIELDS);
    set_message_watchdog(DELETE_DEMO_USERS);
}

function _create_taxonomy()
{
    $terms_data = array('Turkey', 'Egypt', 'Dubai', 'Thailand', 'Spain');
    $get_taxonomy_vocabulary = taxonomy_vocabulary_machine_name_load(TAXONOMY_TRAVEL);

    if (!is_object($get_taxonomy_vocabulary)) {
        taxonomy_vocabulary_save((object)array(
            'name' => 'Travel',
            'machine_name' => TAXONOMY_TRAVEL,
        ));

        $get_taxonomy_vocabulary = taxonomy_vocabulary_machine_name_load(TAXONOMY_TRAVEL);
    } else {
        foreach (taxonomy_get_tree($get_taxonomy_vocabulary->vid) as $term) {
            taxonomy_term_delete($term->tid);
        }
    }

    foreach ($terms_data as $term_data) {
        taxonomy_term_save((object)array(
            'name' => $term_data,
            'vid' => $get_taxonomy_vocabulary->vid,
            'language' => DEMO_LANGUAGE_DEFAULT,
        ));
    }

    set_message_watchdog(TRAVEL_TAXONOMY_ADDED);
}

function _delete_taxonomy()
{
    $get_taxonomy_vocabulary = taxonomy_vocabulary_machine_name_load(TAXONOMY_TRAVEL);
    if (is_object($get_taxonomy_vocabulary)) {
        taxonomy_vocabulary_delete($get_taxonomy_vocabulary->vid);
        set_message_watchdog(TRAVEL_VOCABULARY_DELETED);
    } else {
        set_message_watchdog(TRAVEL_VOCABULARY_NOT_FOUND);
    }
}

function _add_taxonomy_to_travel_node()
{
    $data_fields = array(
        array(
            'field' => array(
                'field_name' => 'field_taxonomy_field',
                'type' => 'taxonomy_term_reference',
            ),
            'instance' => array(
                'bundle' => TAXONOMY_TRAVEL,
                'default_value' => NULL,
                'deleted' => 0,
                'description' => '',
                'display' => array(
                    'default' => array(
                        'label' => 'above',
                        'module' => 'taxonomy',
                        'settings' => array(),
                        'type' => 'taxonomy_term_reference_link',
                        'weight' => 3,
                    ),
                    'teaser' => array(
                        'label' => 'above',
                        'settings' => array(),
                        'type' => 'hidden',
                        'weight' => 0,
                    ),
                ),
                'entity_type' => 'node',
                'field_name' => 'field_taxonomy_field',
                'label' => 'Travel Taxonomy',
                'required' => FALSE,
                'settings' => array(
                    'allowed_values' => array(
                        0 => array(
                            'vocabulary' => array(
                                0 => TAXONOMY_TRAVEL,
                            ),
                        ),
                    ),
                    'user_register_form' => FALSE,
                ),
                'widget' => array(
                    'module' => 'options',
                    'settings' => array(),
                    'type' => 'options_select',
                    'weight' => 4,
                ),
            ),
        ),
    );

    foreach ($data_fields as $value) {
        if (!field_info_field($value['field']['field_name'])) {
            field_create_field($value['field']);
            field_create_instance($value['instance']);
        }
    }
    $field = field_info_field('field_taxonomy_field');
    $field['settings']['allowed_values'][0]['vocabulary'] = TAXONOMY_TRAVEL;
    $field['cardinality'] = 2;
    field_update_field($field);

    $travel_nodes = node_load_multiple(array(), array('type' => TAXONOMY_TRAVEL));

    foreach ($travel_nodes as $node) {
        $load_node = node_load($node->vid);
        foreach (get_random_taxonomy_array() as $taxonomy) {
            $load_node->field_taxonomy_field['und'][]['tid'] = $taxonomy->tid;
        }

        node_save($load_node);
    }

    set_message_watchdog(TRAVEL_TAXONOMY_ADDED_TO_NODE);
}

function get_random_taxonomy_array()
{
    $get_taxonomy_vocabulary = taxonomy_vocabulary_machine_name_load(TAXONOMY_TRAVEL);
    if (is_object($get_taxonomy_vocabulary)) {
        $taxonomy_terms = taxonomy_get_tree($get_taxonomy_vocabulary->vid);
        $rand_terms = (array)array_rand($taxonomy_terms, rand(1, 2));
        $taxonomy_data = array();

        foreach ($rand_terms as $term) {
            $taxonomy_data[] = $taxonomy_terms[$term];
        }

        return $taxonomy_data;
    }
}

function set_message_watchdog($msg)
{
    drupal_set_message(t($msg));
    smartling_watchdog($msg, TRUE, array(), WATCHDOG_INFO);
}

